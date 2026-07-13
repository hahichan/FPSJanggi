// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuthoritativeJanggiBoard.h"

#include "ArenaPlaceholderCharacter.h"
#include "BoardPlayerController.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

namespace
{
constexpr int32 BoardRows = 10;
constexpr int32 BoardColumns = 9;
// Exact spacing used by origin/master's JanggiBoard1 "index -> position" macro.
constexpr float BoardRowSpacing = 8.0f;
constexpr float BoardColumnSpacing = 8.5f;
constexpr int32 MaximumReplicatedClicks = 2048;

UClass* GetJanggiPieceBaseClass()
{
	static UClass* PieceBaseClass = LoadClass<AActor>(
		nullptr, TEXT("/Game/User/Blueprints/Piece/BP_Piece_Base.BP_Piece_Base_C"));
	return PieceBaseClass;
}

void GetAllJanggiPieces(const UObject* WorldContext, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (UClass* PieceBaseClass = GetJanggiPieceBaseClass())
	{
		UGameplayStatics::GetAllActorsOfClass(WorldContext, PieceBaseClass, OutActors);
	}
}

int32 BoardIndexOf(int32 Row, int32 Column)
{
	return Row * BoardColumns + Column;
}

bool IsInsideBoard(int32 Row, int32 Column)
{
	return Row >= 0 && Row < BoardRows && Column >= 0 && Column < BoardColumns;
}

const TCHAR* GetInitialPieceClassPath(int32 Value)
{
	switch (Value)
	{
	case 1: return TEXT("/Game/User/Blueprints/Piece/BP_Solder_Blue.BP_Solder_Blue_C");
	case 2: return TEXT("/Game/User/Blueprints/Piece/BP_Cannon_Blue.BP_Cannon_Blue_C");
	case 3: return TEXT("/Game/User/Blueprints/Piece/BP_Chariot_Blue.BP_Chariot_Blue_C");
	case 4: return TEXT("/Game/User/Blueprints/Piece/BP_Elephant_Blue.BP_Elephant_Blue_C");
	case 5: return TEXT("/Game/User/Blueprints/Piece/BP_Horse_Blue.BP_Horse_Blue_C");
	case 6: return TEXT("/Game/User/Blueprints/Piece/BP_Guard_Blue.BP_Guard_Blue_C");
	case 7: return TEXT("/Game/User/Blueprints/Piece/BP_Genera_Blue.BP_Genera_Blue_C");
	case 11: return TEXT("/Game/User/Blueprints/Piece/BP_Solder_Red.BP_Solder_Red_C");
	case 12: return TEXT("/Game/User/Blueprints/Piece/BP_Cannon_Red.BP_Cannon_Red_C");
	case 13: return TEXT("/Game/User/Blueprints/Piece/BP_Chariot_Red.BP_Chariot_Red_C");
	case 14: return TEXT("/Game/User/Blueprints/Piece/BP_Elephant_Red.BP_Elephant_Red_C");
	case 15: return TEXT("/Game/User/Blueprints/Piece/BP_Horse_Red.BP_Horse_Red_C");
	case 16: return TEXT("/Game/User/Blueprints/Piece/BP_Guard_Red.BP_Guard_Red_C");
	case 17: return TEXT("/Game/User/Blueprints/Piece/BP_Genera_Red.BP_Genera_Red_C");
	default: return nullptr;
	}
}

FString NormalizePropertyName(FString Name)
{
	Name.ReplaceInline(TEXT(" "), TEXT(""));
	Name.ReplaceInline(TEXT("_"), TEXT(""));
	return Name.ToLower();
}

template <typename PropertyType>
PropertyType* FindPropertyNormalized(const UClass* Class, const TCHAR* RequestedName)
{
	const FString Requested = NormalizePropertyName(RequestedName);
	for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (NormalizePropertyName(It->GetName()) == Requested)
		{
			return CastField<PropertyType>(*It);
		}
	}
	return nullptr;
}
}

AAuthoritativeJanggiBoard::AAuthoritativeJanggiBoard()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
	NetDormancy = DORM_Awake;
}

void AAuthoritativeJanggiBoard::BeginPlay()
{
	Super::BeginPlay();
	BeginPieceVisualSynchronization();
	if (HasAuthority())
	{
		bBoardInputPaused = true;
		TurnDeadlineServerTime = 0.0;
	}
}

void AAuthoritativeJanggiBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, ClickHistory);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, bBoardInputPaused);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, CurrentTurnTeam);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, MatchPhase);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, WinningTeam);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, BattleContext);
	DOREPLIFETIME(AAuthoritativeJanggiBoard, TurnDeadlineServerTime);
}

bool AAuthoritativeJanggiBoard::SubmitAuthoritativeClick(
	APlayerController* RequestingController,
	const FVector& LocalPosition)
{
	ABoardPlayerController* Controller = Cast<ABoardPlayerController>(RequestingController);
	if (IsValid(Controller) && Controller->GetAssignedBoardTeam() != CurrentTurnTeam)
	{
		Controller->ClientShowBoardNotice(
			FString::Printf(TEXT("현재 %s 차례입니다"),
				CurrentTurnTeam == EJanggiTeam::Blue ? TEXT("청") : TEXT("한")),
			true);
	}
	if (!HasAuthority() || bBoardInputPaused || MatchPhase != EBoardMatchPhase::BoardTurn ||
		!IsValid(Controller) || Controller->GetWorld() != GetWorld() ||
		Controller->GetAssignedBoardTeam() != CurrentTurnTeam ||
		ClickHistory.Num() >= MaximumReplicatedClicks || !IsValidBoardLocalPosition(LocalPosition))
	{
		UE_LOG(LogTemp, Verbose, TEXT("BOARD_REQUEST_REJECTED phase_or_owner"));
		return false;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (LastServerClickTime >= 0.0 && Now - LastServerClickTime < 0.08)
	{
		return false;
	}
	LastServerClickTime = Now;

	const FVector SanitizedPosition(
		FMath::RoundToDouble(LocalPosition.X / BoardRowSpacing) * BoardRowSpacing,
		FMath::RoundToDouble(LocalPosition.Y / BoardColumnSpacing) * BoardColumnSpacing,
		0.0);
	const int32 ClickedIndex = GetBoardIndex(SanitizedPosition);
	AActor* ClickedPiece = GetPieceAt(ClickedIndex);

	if (SelectedIndex == INDEX_NONE)
	{
		if (!ClickedPiece || GetPieceTeam(ClickedPiece) != CurrentTurnTeam)
		{
			Controller->ClientShowBoardNotice(
				CurrentTurnTeam == EJanggiTeam::Blue
					? TEXT("청 기물을 선택하세요")
					: TEXT("한 기물을 선택하세요"),
				true);
			UE_LOG(LogTemp, Display, TEXT("BOARD_REQUEST_REJECTED invalid_selection index=%d"), ClickedIndex);
			return false;
		}
		if (MaxConsecutiveMovesPerPiece > 0 && LastMovedPiece.Get() == ClickedPiece &&
			ConsecutiveMovesForPiece >= MaxConsecutiveMovesPerPiece)
		{
			Controller->ClientShowBoardNotice(TEXT("이 기물은 연속 이동 제한에 도달했습니다"), true);
			UE_LOG(LogTemp, Display, TEXT("BOARD_REQUEST_REJECTED piece_limit index=%d limit=%d"), ClickedIndex, MaxConsecutiveMovesPerPiece);
			return false;
		}

		SelectedIndex = ClickedIndex;
		ClickHistory.Add(FVector_NetQuantize10(SanitizedPosition));
		ForceNetUpdate();
		SendLegalDestinations(Controller, SelectedIndex);
		Controller->ClientShowBoardNotice(TEXT("기물 선택됨 · 초록색 칸을 선택하세요"), false);
		UE_LOG(LogTemp, Display, TEXT("BOARD_SELECTION_ACCEPTED team=%s index=%d"), *UEnum::GetValueAsString(CurrentTurnTeam), ClickedIndex);
		return true;
	}

	if (ClickedPiece && GetPieceTeam(ClickedPiece) == CurrentTurnTeam)
	{
		SelectedIndex = ClickedIndex;
		ClickHistory.Add(FVector_NetQuantize10(SanitizedPosition));
		ForceNetUpdate();
		SendLegalDestinations(Controller, SelectedIndex);
		Controller->ClientShowBoardNotice(TEXT("선택 변경됨 · 초록색 칸을 선택하세요"), false);
		return true;
	}

	AActor* SelectedPiece = GetPieceAt(SelectedIndex);
	if (!SelectedPiece || GetPieceTeam(SelectedPiece) != CurrentTurnTeam ||
		!IsLegalMove(SelectedIndex, ClickedIndex, GetPieceType(SelectedPiece), CurrentTurnTeam))
	{
		Controller->ClientShowBoardNotice(TEXT("이동할 수 없는 칸입니다"), true);
		UE_LOG(LogTemp, Display, TEXT("BOARD_REQUEST_REJECTED illegal_move from=%d to=%d type=%s"),
			SelectedIndex, ClickedIndex, *UEnum::GetValueAsString(GetPieceType(SelectedPiece)));
		return false;
	}

	if (ClickedPiece)
	{
		UE_LOG(LogTemp, Display, TEXT("BOARD_BATTLE_REQUEST from=%d to=%d"), SelectedIndex, ClickedIndex);
		StartArenaBattle(ClickedIndex);
		return true;
	}

	CompleteBoardMove(ClickedIndex);
	UE_LOG(LogTemp, Display, TEXT("BOARD_MOVE_ACCEPTED destination=%d"), ClickedIndex);
	StartTurn(CurrentTurnTeam == EJanggiTeam::Blue ? EJanggiTeam::Red : EJanggiTeam::Blue);
	return true;
}

bool AAuthoritativeJanggiBoard::RunFirstLegalMoveForTesting(APlayerController* RequestingController)
{
#if !UE_BUILD_SHIPPING
	const ABoardPlayerController* Controller = Cast<ABoardPlayerController>(RequestingController);
	if (!bBoardInitializationRequested)
	{
		SelectFormation(RequestingController, EJanggiFormation::HorseElephantElephantHorse);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ABoardPlayerController* Other = Cast<ABoardPlayerController>(It->Get());
			if (Other && Other != RequestingController && Other->GetAssignedBoardTeam() == EJanggiTeam::Red)
			{
				SelectFormation(Other, EJanggiFormation::ElephantHorseHorseElephant);
				break;
			}
		}
	}
	int32 FromIndex = INDEX_NONE;
	int32 DestinationIndex = INDEX_NONE;
	if (!Controller || !FindFirstLegalNonCaptureMove(Controller->GetAssignedBoardTeam(), FromIndex, DestinationIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_SMOKE_TEST no legal non-capture move found"));
		return false;
	}
	LastServerClickTime = -1.0;
	const bool bSelected = SubmitAuthoritativeClick(RequestingController, GetCellLocalPosition(FromIndex));
	LastServerClickTime = -1.0;
	const bool bMoved = bSelected && SubmitAuthoritativeClick(RequestingController, GetCellLocalPosition(DestinationIndex));
	UE_LOG(LogTemp, Display, TEXT("BOARD_SMOKE_TEST from=%d to=%d selected=%s moved=%s"),
		FromIndex, DestinationIndex, bSelected ? TEXT("true") : TEXT("false"), bMoved ? TEXT("true") : TEXT("false"));
	return bMoved;
#else
	return false;
#endif
}

bool AAuthoritativeJanggiBoard::RunFirstArenaBattleForTesting(APlayerController* RequestingController)
{
#if !UE_BUILD_SHIPPING
	const ABoardPlayerController* Controller = Cast<ABoardPlayerController>(RequestingController);
	if (!bBoardInitializationRequested)
	{
		SelectFormation(RequestingController, EJanggiFormation::HorseElephantElephantHorse);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ABoardPlayerController* Other = Cast<ABoardPlayerController>(It->Get());
			if (Other && Other != RequestingController && Other->GetAssignedBoardTeam() == EJanggiTeam::Red)
			{
				SelectFormation(Other, EJanggiFormation::ElephantHorseHorseElephant);
				break;
			}
		}
	}
	int32 FromIndex = INDEX_NONE;
	int32 DestinationIndex = INDEX_NONE;
	if (!Controller || !FindFirstLegalNonCaptureMove(Controller->GetAssignedBoardTeam(), FromIndex, DestinationIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_ARENA_SMOKE_TEST no_attacker_move"));
		return false;
	}
	int32 DefenderIndex = INDEX_NONE;
	AActor* Defender = nullptr;
	int32 BlueCount = 0;
	int32 RedCount = 0;
	int32 OtherCount = 0;
	for (int32 Index = 0; Index < BoardRows * BoardColumns; ++Index)
	{
		AActor* Candidate = GetPieceAt(Index);
		if (Candidate)
		{
			const EJanggiTeam CandidateTeam = GetPieceTeam(Candidate);
			if (CandidateTeam == EJanggiTeam::Blue) ++BlueCount;
			else if (CandidateTeam == EJanggiTeam::Red) ++RedCount;
			else ++OtherCount;
		}
		if (Candidate && GetPieceTeam(Candidate) != Controller->GetAssignedBoardTeam())
		{
			DefenderIndex = Index;
			Defender = Candidate;
			break;
		}
	}
	if (!Defender)
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_ARENA_SMOKE_TEST no_defender blue=%d red=%d other=%d controller=%s"),
			BlueCount, RedCount, OtherCount, *UEnum::GetValueAsString(Controller->GetAssignedBoardTeam()));
		return false;
	}
	FArrayProperty* ArrayProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("PieceMesh"));
	FObjectPropertyBase* ObjectProperty = ArrayProperty ? CastField<FObjectPropertyBase>(ArrayProperty->Inner) : nullptr;
	if (!ArrayProperty || !ObjectProperty) return false;
	void* Address = ArrayProperty->ContainerPtrToValuePtr<void>(this);
	FScriptArrayHelper Array(ArrayProperty, Address);
	ObjectProperty->SetObjectPropertyValue(Array.GetRawPtr(DefenderIndex), nullptr);
	ObjectProperty->SetObjectPropertyValue(Array.GetRawPtr(DestinationIndex), Defender);
	Defender->SetActorLocation(GetActorTransform().TransformPosition(GetCellLocalPosition(DestinationIndex)), false, nullptr, ETeleportType::TeleportPhysics);
	LastServerClickTime = -1.0;
	const bool bSelected = SubmitAuthoritativeClick(RequestingController, GetCellLocalPosition(FromIndex));
	LastServerClickTime = -1.0;
	const bool bBattleStarted = bSelected && SubmitAuthoritativeClick(RequestingController, GetCellLocalPosition(DestinationIndex));
	UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_SMOKE_TEST from=%d to=%d started=%s"),
		FromIndex, DestinationIndex, bBattleStarted ? TEXT("true") : TEXT("false"));
	return bBattleStarted;
#else
	return false;
#endif
}

bool AAuthoritativeJanggiBoard::RunGeneralDefeatForTesting(APlayerController* RequestingController)
{
#if !UE_BUILD_SHIPPING
	ABoardPlayerController* Controller = Cast<ABoardPlayerController>(RequestingController);
	if (!Controller) return false;
	if (!bBoardInitializationRequested)
	{
		SelectFormation(RequestingController, EJanggiFormation::HorseElephantElephantHorse);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ABoardPlayerController* Other = Cast<ABoardPlayerController>(It->Get());
			if (Other && Other != RequestingController && Other->GetAssignedBoardTeam() == EJanggiTeam::Red)
			{
				SelectFormation(Other, EJanggiFormation::ElephantHorseHorseElephant);
				break;
			}
		}
	}
	RebuildPieceCache();
	int32 AttackerIndex = INDEX_NONE;
	int32 GeneralIndex = INDEX_NONE;
	for (int32 Index = 0; Index < BoardRows * BoardColumns; ++Index)
	{
		AActor* Piece = GetPieceAt(Index);
		if (!Piece) continue;
		if (AttackerIndex == INDEX_NONE && GetPieceTeam(Piece) == EJanggiTeam::Blue &&
			GetPieceType(Piece) != EJanggiPieceType::General)
		{
			AttackerIndex = Index;
		}
		if (GetPieceTeam(Piece) == EJanggiTeam::Red && GetPieceType(Piece) == EJanggiPieceType::General)
		{
			GeneralIndex = Index;
		}
	}
	if (AttackerIndex == INDEX_NONE || GeneralIndex == INDEX_NONE) return false;
	CurrentTurnTeam = EJanggiTeam::Blue;
	SelectedIndex = AttackerIndex;
	bBoardInputPaused = true;
	MatchPhase = EBoardMatchPhase::ArenaBattle;
	BattleContext.bActive = true;
	BattleContext.OriginIndex = AttackerIndex;
	BattleContext.DestinationIndex = GeneralIndex;
	BattleContext.AttackerTeam = EJanggiTeam::Blue;
	BattleContext.AttackerType = GetPieceType(GetPieceAt(AttackerIndex));
	BattleContext.DefenderType = EJanggiPieceType::General;
	BattleContext.Attacker = GetPieceAt(AttackerIndex);
	BattleContext.Defender = GetPieceAt(GeneralIndex);
	ResolveArenaBattle(EJanggiTeam::Blue);
	const bool bFinished = MatchPhase == EBoardMatchPhase::MatchFinished && WinningTeam == EJanggiTeam::Blue;
	UE_LOG(LogTemp, Display, TEXT("BOARD_GENERAL_SMOKE_TEST finished=%s winner=%s"),
		bFinished ? TEXT("true") : TEXT("false"), *UEnum::GetValueAsString(WinningTeam));
	return bFinished;
#else
	return false;
#endif
}

void AAuthoritativeJanggiBoard::EnsureBoardInitialized()
{
	if (!HasAuthority() || bBoardInitializationRequested) return;
	RebuildPieceCache();
	for (const TWeakObjectPtr<AActor>& Piece : PieceCache)
		if (Piece.IsValid()) return;

	bBoardInitializationRequested = true;
	const FArrayProperty* StartProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("StartFigure"));
	const FIntProperty* ValueProperty = StartProperty ? CastField<FIntProperty>(StartProperty->Inner) : nullptr;
	if (StartProperty && ValueProperty)
	{
		const void* Address = StartProperty->ContainerPtrToValuePtr<void>(this);
		FScriptArrayHelper Values(StartProperty, Address);
		FArrayProperty* PieceArrayProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("PieceMesh"));
		FObjectPropertyBase* PieceObjectProperty = PieceArrayProperty
			? CastField<FObjectPropertyBase>(PieceArrayProperty->Inner) : nullptr;
		if (!PieceArrayProperty || !PieceObjectProperty)
		{
			UE_LOG(LogTemp, Error, TEXT("BOARD_INITIALIZATION invalid PieceMesh array"));
			return;
		}
		void* PieceAddress = PieceArrayProperty->ContainerPtrToValuePtr<void>(this);
		FScriptArrayHelper PieceArray(PieceArrayProperty, PieceAddress);
		PieceArray.Resize(BoardRows * BoardColumns);
		PieceCache.SetNum(BoardRows * BoardColumns);
		for (TWeakObjectPtr<AActor>& Entry : PieceCache)
		{
			Entry.Reset();
		}
		int32 SpawnedCount = 0;
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			const int32 Value = ValueProperty->GetPropertyValue(Values.GetRawPtr(Index));
			AActor* Piece = SpawnAuthoritativePiece(Value, Index);
			if (!Piece) continue;
			PieceObjectProperty->SetObjectPropertyValue(PieceArray.GetRawPtr(Index), Piece);
			if (PieceCache.IsValidIndex(Index)) PieceCache[Index] = Piece;
			++SpawnedCount;
		}
		UE_LOG(LogTemp, Display, TEXT("BOARD_INITIALIZATION spawned=%d piece_array=%d"), SpawnedCount, PieceArray.Num());
		ForceNetUpdate();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_INITIALIZATION missing Start_Figure array"));
	}
}

AActor* AAuthoritativeJanggiBoard::SpawnAuthoritativePiece(int32 PieceValue, int32 BoardIndex)
{
	const TCHAR* ClassPath = GetInitialPieceClassPath(PieceValue);
	if (!ClassPath) return nullptr;
	UClass* PieceClass = LoadClass<AActor>(nullptr, ClassPath);
	if (!PieceClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_INITIALIZATION failed class=%s"), ClassPath);
		return nullptr;
	}

	const bool bRedPiece = PieceValue >= 11;
	FTransform SpawnTransform(
		GetActorRotation() + FRotator(0.0, bRedPiece ? 270.0 : 90.0, 0.0),
		GetActorTransform().TransformPosition(GetCellLocalPosition(BoardIndex)),
		FVector::OneVector);
	AActor* Piece = GetWorld()->SpawnActorDeferred<AActor>(
		PieceClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Piece) return nullptr;
	Piece->SetReplicates(true);
	Piece->SetReplicateMovement(true);
	UGameplayStatics::FinishSpawningActor(Piece, SpawnTransform);
	ApplyPieceVisualScale(Piece);
	return Piece;
}

bool AAuthoritativeJanggiBoard::SelectFormation(
	APlayerController* RequestingController, EJanggiFormation Formation)
{
	ABoardPlayerController* Controller = Cast<ABoardPlayerController>(RequestingController);
	if (!HasAuthority() || !Controller || bBoardInitializationRequested || ClickHistory.Num() > 0 ||
		MatchPhase != EBoardMatchPhase::BoardTurn) return false;
	const EJanggiTeam Team = Controller->GetAssignedBoardTeam();
	if (Team != EJanggiTeam::Blue && Team != EJanggiTeam::Red) return false;

	if (Team == EJanggiTeam::Blue)
	{
		if (bBlueFormationSelected) return false;
		BlueFormation = Formation;
		bBlueFormationSelected = true;
	}
	else
	{
		if (bRedFormationSelected) return false;
		RedFormation = Formation;
		bRedFormationSelected = true;
	}

	UE_LOG(LogTemp, Display, TEXT("BOARD_FORMATION_PENDING team=%s formation=%s blue=%s red=%s"),
		*UEnum::GetValueAsString(Team), *UEnum::GetValueAsString(Formation),
		bBlueFormationSelected ? TEXT("true") : TEXT("false"),
		bRedFormationSelected ? TEXT("true") : TEXT("false"));
	Controller->ClientMessage(TEXT("진형 선택 완료. 상대 플레이어를 기다리는 중입니다."));
	if (!bBlueFormationSelected || !bRedFormationSelected)
	{
		ForceNetUpdate();
		return true;
	}

	if (!ApplyFormationToStartArray(EJanggiTeam::Blue, BlueFormation) ||
		!ApplyFormationToStartArray(EJanggiTeam::Red, RedFormation))
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_FORMATION_START_ARRAY_FAILED"));
		return false;
	}
	EnsureBoardInitialized();
	WinningTeam = EJanggiTeam::Unassigned;
	bBoardInputPaused = false;
	StartTurn(EJanggiTeam::Blue);
	BeginPieceVisualSynchronization();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* Player = It->Get())
			Player->ClientMessage(TEXT("양쪽 진형 선택 완료. 청 팀의 턴입니다."));
	}
	UE_LOG(LogTemp, Display, TEXT("BOARD_MATCH_STARTED blue=%s red=%s"),
		*UEnum::GetValueAsString(BlueFormation), *UEnum::GetValueAsString(RedFormation));
	ForceNetUpdate();
	return true;
}

bool AAuthoritativeJanggiBoard::ApplyFormationToStartArray(
	EJanggiTeam Team, EJanggiFormation Formation)
{
	FArrayProperty* StartProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("StartFigure"));
	FIntProperty* ValueProperty = StartProperty ? CastField<FIntProperty>(StartProperty->Inner) : nullptr;
	if (!StartProperty || !ValueProperty) return false;
	void* Address = StartProperty->ContainerPtrToValuePtr<void>(this);
	FScriptArrayHelper StartValues(StartProperty, Address);
	if (StartValues.Num() < BoardRows * BoardColumns) return false;

	const bool bBlue = Team == EJanggiTeam::Blue;
	const int32 Indices[4] = { bBlue ? 1 : 82, bBlue ? 2 : 83, bBlue ? 6 : 87, bBlue ? 7 : 88 };
	const int32 Horse = bBlue ? 5 : 15;
	const int32 Elephant = bBlue ? 4 : 14;
	int32 Values[4] = { Horse, Elephant, Elephant, Horse };
	if (Formation == EJanggiFormation::HorseElephantHorseElephant)
	{
		Values[0] = Horse; Values[1] = Elephant; Values[2] = Horse; Values[3] = Elephant;
	}
	else if (Formation == EJanggiFormation::ElephantHorseElephantHorse)
	{
		Values[0] = Elephant; Values[1] = Horse; Values[2] = Elephant; Values[3] = Horse;
	}
	else if (Formation == EJanggiFormation::ElephantHorseHorseElephant)
	{
		Values[0] = Elephant; Values[1] = Horse; Values[2] = Horse; Values[3] = Elephant;
	}
	for (int32 Slot = 0; Slot < 4; ++Slot)
		ValueProperty->SetPropertyValue(StartValues.GetRawPtr(Indices[Slot]), Values[Slot]);
	return true;
}

void AAuthoritativeJanggiBoard::SetBoardInputPaused(bool bPaused)
{
	if (HasAuthority())
	{
		bBoardInputPaused = bPaused;
		ForceNetUpdate();
	}
}

void AAuthoritativeJanggiBoard::ResolveArenaBattle(EJanggiTeam WinnerTeam)
{
	if (!HasAuthority() || !BattleContext.bActive ||
		(MatchPhase != EBoardMatchPhase::ArenaBattle && MatchPhase != EBoardMatchPhase::ArenaTransition))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(ArenaTransitionTimerHandle);
	GetWorldTimerManager().ClearTimer(ArenaBattleTimerHandle);
	MatchPhase = EBoardMatchPhase::BattleResolution;
	UE_LOG(LogTemp, Display, TEXT("BOARD_BATTLE_RESOLVED winner=%s"), *UEnum::GetValueAsString(WinnerTeam));
	const FBoardBattleContext ResolvedContext = BattleContext;
	OnArenaBattleResolved(ResolvedContext, WinnerTeam);
	const bool bAttackerWon = WinnerTeam == ResolvedContext.AttackerTeam;
	const bool bGeneralDefeated =
		(bAttackerWon && ResolvedContext.DefenderType == EJanggiPieceType::General) ||
		(!bAttackerWon && ResolvedContext.AttackerType == EJanggiPieceType::General);

	if (bAttackerWon)
	{
		CompleteBoardMove(ResolvedContext.DestinationIndex);
	}
	else
	{
		const bool bAttackerRemoved = RemoveBoardPieceAt(ResolvedContext.OriginIndex);
		ClickHistory.Add(FVector_NetQuantize10(GetCellLocalPosition(ResolvedContext.OriginIndex)));
		SelectedIndex = INDEX_NONE;
		ClearLegalMoveMarkersForAllPlayers();
		UE_LOG(LogTemp, Display, TEXT("BOARD_BATTLE_LOSER_REMOVED team=%s index=%d removed=%s role=attacker"),
			*UEnum::GetValueAsString(ResolvedContext.AttackerTeam), ResolvedContext.OriginIndex,
			bAttackerRemoved ? TEXT("true") : TEXT("false"));
	}

	NotifyArenaCamera(false);
	DestroyArenaPlaceholderCharacters();
	BattleContext = FBoardBattleContext();
	if (bGeneralDefeated)
	{
		FinishMatch(WinnerTeam);
	}
	else
	{
		bBoardInputPaused = false;
		StartTurn(CurrentTurnTeam == EJanggiTeam::Blue ? EJanggiTeam::Red : EJanggiTeam::Blue);
	}
	ForceNetUpdate();
}

void AAuthoritativeJanggiBoard::FinishMatch(EJanggiTeam WinnerTeam)
{
	if (!HasAuthority() || (WinnerTeam != EJanggiTeam::Blue && WinnerTeam != EJanggiTeam::Red)) return;
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	GetWorldTimerManager().ClearTimer(ArenaTransitionTimerHandle);
	GetWorldTimerManager().ClearTimer(ArenaBattleTimerHandle);
	ClearLegalMoveMarkersForAllPlayers();
	ClearExistingBlueprintSelection();
	SelectedIndex = INDEX_NONE;
	bBoardInputPaused = true;
	TurnDeadlineServerTime = 0.0;
	WinningTeam = WinnerTeam;
	MatchPhase = EBoardMatchPhase::MatchFinished;
	UE_LOG(LogTemp, Display, TEXT("BOARD_MATCH_FINISHED winner=%s reason=general_defeated"),
		*UEnum::GetValueAsString(WinningTeam));
	ForceNetUpdate();
}

float AAuthoritativeJanggiBoard::GetRemainingTurnSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World) return 0.0f;
	const AGameStateBase* GameState = World->GetGameState();
	const double ServerTime = GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
	return FMath::Max(0.0, TurnDeadlineServerTime - ServerTime);
}

void AAuthoritativeJanggiBoard::OnRep_ClickHistory()
{
	BeginPieceVisualSynchronization();
	if (SynchronizeBlueprintPieceArrayFromWorld() > 0 || ClickHistory.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(PieceSynchronizationTimerHandle);
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			PieceSynchronizationTimerHandle, this, &AAuthoritativeJanggiBoard::OnRep_ClickHistory, 0.25f, false);
	}
}

void AAuthoritativeJanggiBoard::OnRep_BattleContext()
{
	if (BattleContext.bActive)
	{
		OnArenaBattleStarted(BattleContext);
	}
}

bool AAuthoritativeJanggiBoard::IsValidBoardLocalPosition(const FVector& LocalPosition) const
{
	return !LocalPosition.ContainsNaN() &&
		LocalPosition.X >= -BoardRowSpacing * 0.5f &&
		LocalPosition.X < (BoardRows - 0.5f) * BoardRowSpacing &&
		LocalPosition.Y >= -BoardColumnSpacing * 0.5f &&
		LocalPosition.Y < (BoardColumns - 0.5f) * BoardColumnSpacing &&
		FMath::Abs(LocalPosition.Z) <= 500.0f;
}

int32 AAuthoritativeJanggiBoard::GetBoardIndex(const FVector& SanitizedLocalPosition) const
{
	return BoardIndexOf(
		FMath::RoundToInt32(SanitizedLocalPosition.X / BoardRowSpacing),
		FMath::RoundToInt32(SanitizedLocalPosition.Y / BoardColumnSpacing));
}

FVector AAuthoritativeJanggiBoard::GetCellLocalPosition(int32 BoardIndex) const
{
	return FVector(
		(BoardIndex / BoardColumns) * BoardRowSpacing,
		(BoardIndex % BoardColumns) * BoardColumnSpacing,
		0.0);
}

FVector AAuthoritativeJanggiBoard::GetCellWorldPosition(int32 BoardIndex) const
{
	if (BoardIndex < 0 || BoardIndex >= BoardRows * BoardColumns)
	{
		return GetActorLocation();
	}
	return GetActorTransform().TransformPosition(GetCellLocalPosition(BoardIndex));
}

TArray<int32> AAuthoritativeJanggiBoard::GetLegalDestinations(int32 FromIndex, EJanggiTeam Team) const
{
	TArray<int32> Result;
	AActor* Piece = GetPieceAt(FromIndex);
	if (!Piece || GetPieceTeam(Piece) != Team)
	{
		return Result;
	}

	const EJanggiPieceType PieceType = GetPieceType(Piece);
	Result.Reserve(16);
	for (int32 Destination = 0; Destination < BoardRows * BoardColumns; ++Destination)
	{
		if (IsLegalMove(FromIndex, Destination, PieceType, Team))
		{
			Result.Add(Destination);
		}
	}
	return Result;
}

void AAuthoritativeJanggiBoard::SendLegalDestinations(ABoardPlayerController* Controller, int32 FromIndex) const
{
	if (!Controller)
	{
		return;
	}
	Controller->ClientShowLegalMoves(GetLegalDestinations(FromIndex, Controller->GetAssignedBoardTeam()));
}

void AAuthoritativeJanggiBoard::ClearLegalMoveMarkersForAllPlayers() const
{
	if (!GetWorld())
	{
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get()))
		{
			Controller->ClientClearLegalMoves();
		}
	}
}

AActor* AAuthoritativeJanggiBoard::GetPieceAt(int32 BoardIndex) const
{
	const FArrayProperty* ArrayProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("PieceMesh"));
	const FObjectPropertyBase* ObjectProperty = ArrayProperty
		? CastField<FObjectPropertyBase>(ArrayProperty->Inner) : nullptr;
	if (ArrayProperty && ObjectProperty)
	{
		const void* ArrayAddress = ArrayProperty->ContainerPtrToValuePtr<void>(this);
		FScriptArrayHelper Array(ArrayProperty, ArrayAddress);
		if (Array.IsValidIndex(BoardIndex))
		{
			if (AActor* Piece = Cast<AActor>(ObjectProperty->GetObjectPropertyValue(Array.GetRawPtr(BoardIndex))))
			{
				return Piece;
			}
		}
	}
	return PieceCache.IsValidIndex(BoardIndex) ? PieceCache[BoardIndex].Get() : nullptr;
}

void AAuthoritativeJanggiBoard::RebuildPieceCache() const
{
	PieceCache.SetNum(BoardRows * BoardColumns);
	for (TWeakObjectPtr<AActor>& Entry : PieceCache) Entry.Reset();
	TArray<AActor*> Actors;
	GetAllJanggiPieces(this, Actors);
	for (AActor* Actor : Actors)
	{
		if (!Actor || Actor == this) continue;
		const FVector Local = GetActorTransform().InverseTransformPosition(Actor->GetActorLocation());
		const int32 Row = FMath::RoundToInt32(Local.X / BoardRowSpacing);
		const int32 Column = FMath::RoundToInt32(Local.Y / BoardColumnSpacing);
		if (IsInsideBoard(Row, Column)) PieceCache[BoardIndexOf(Row, Column)] = Actor;
	}
}

int32 AAuthoritativeJanggiBoard::SynchronizeBlueprintPieceArrayFromWorld()
{
	RebuildPieceCache();
	FArrayProperty* ArrayProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("PieceMesh"));
	FObjectPropertyBase* ObjectProperty = ArrayProperty ? CastField<FObjectPropertyBase>(ArrayProperty->Inner) : nullptr;
	if (!ArrayProperty || !ObjectProperty) return 0;
	void* Address = ArrayProperty->ContainerPtrToValuePtr<void>(this);
	FScriptArrayHelper Array(ArrayProperty, Address);
	Array.Resize(BoardRows * BoardColumns);
	int32 Count = 0;
	for (int32 Index = 0; Index < PieceCache.Num(); ++Index)
	{
		if (AActor* Piece = PieceCache[Index].Get())
		{
			ObjectProperty->SetObjectPropertyValue(Array.GetRawPtr(Index), Piece);
			++Count;
		}
	}
	return Count;
}

void AAuthoritativeJanggiBoard::BeginPieceVisualSynchronization()
{
	PieceVisualSynchronizationAttempts = 0;
	GetWorldTimerManager().ClearTimer(PieceVisualSynchronizationTimerHandle);
	GetWorldTimerManager().SetTimer(
		PieceVisualSynchronizationTimerHandle,
		this,
		&AAuthoritativeJanggiBoard::SynchronizePieceVisualsFromWorld,
		0.25f,
		true,
		0.0f);
}

void AAuthoritativeJanggiBoard::SynchronizePieceVisualsFromWorld()
{
	TArray<AActor*> Actors;
	GetAllJanggiPieces(this, Actors);
	int32 SynchronizedCount = 0;
	float MinimumScale = TNumericLimits<float>::Max();
	float MaximumScale = 0.0f;
	for (AActor* Actor : Actors)
	{
		if (!Actor || Actor == this) continue;
		ApplyPieceVisualScale(Actor);
		const float Scale = Actor->GetActorScale3D().X;
		MinimumScale = FMath::Min(MinimumScale, Scale);
		MaximumScale = FMath::Max(MaximumScale, Scale);
		++SynchronizedCount;
	}
	if (SynchronizedCount >= 32 || ++PieceVisualSynchronizationAttempts >= 20)
	{
		GetWorldTimerManager().ClearTimer(PieceVisualSynchronizationTimerHandle);
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Display, TEXT("BOARD_VISUALS_READY role=%s pieces=%d scale_min=%.2f scale_max=%.2f"),
			*UEnum::GetValueAsString(GetLocalRole()), SynchronizedCount,
			SynchronizedCount > 0 ? MinimumScale : 0.0f, MaximumScale);
#endif
	}
}

void AAuthoritativeJanggiBoard::ApplyPieceVisualScale(AActor* Piece) const
{
	if (!Piece) return;
	// origin/master spawned every piece with a fixed (1.8, 1.8, 1.8) transform scale.
	Piece->SetActorScale3D(FVector(FMath::Max(0.01f, PieceSpawnScale)));
}

EJanggiTeam AAuthoritativeJanggiBoard::GetPieceTeam(const AActor* Piece) const
{
	if (!Piece)
	{
		return EJanggiTeam::Unassigned;
	}
	const FString Name = Piece->GetClass()->GetPathName().ToLower();
	if (Name.Contains(TEXT("_blue.")) || Name.EndsWith(TEXT("_blue_c"))) return EJanggiTeam::Blue;
	if (Name.Contains(TEXT("_red.")) || Name.EndsWith(TEXT("_red_c"))) return EJanggiTeam::Red;
	return EJanggiTeam::Unassigned;
}

EJanggiPieceType AAuthoritativeJanggiBoard::GetPieceType(const AActor* Piece) const
{
	if (!Piece) return EJanggiPieceType::None;
	const FString Name = Piece->GetClass()->GetPathName().ToLower();
	if (Name.Contains(TEXT("solder")) || Name.Contains(TEXT("soldier"))) return EJanggiPieceType::Soldier;
	if (Name.Contains(TEXT("cannon"))) return EJanggiPieceType::Cannon;
	if (Name.Contains(TEXT("chariot"))) return EJanggiPieceType::Chariot;
	if (Name.Contains(TEXT("elephant"))) return EJanggiPieceType::Elephant;
	if (Name.Contains(TEXT("genera"))) return EJanggiPieceType::General;
	if (Name.Contains(TEXT("guard"))) return EJanggiPieceType::Guard;
	if (Name.Contains(TEXT("horse"))) return EJanggiPieceType::Horse;
	return EJanggiPieceType::None;
}

bool AAuthoritativeJanggiBoard::IsLegalMove(
	int32 FromIndex, int32 ToIndex, EJanggiPieceType PieceType, EJanggiTeam Team) const
{
	if (FromIndex == ToIndex || FromIndex < 0 || ToIndex < 0 || FromIndex >= 90 || ToIndex >= 90)
	{
		return false;
	}
	AActor* DestinationPiece = GetPieceAt(ToIndex);
	if (DestinationPiece && GetPieceTeam(DestinationPiece) == Team)
	{
		return false;
	}

	const int32 FR = FromIndex / BoardColumns;
	const int32 FC = FromIndex % BoardColumns;
	const int32 TR = ToIndex / BoardColumns;
	const int32 TC = ToIndex % BoardColumns;
	const int32 DR = TR - FR;
	const int32 DC = TC - FC;
	const int32 AR = FMath::Abs(DR);
	const int32 AC = FMath::Abs(DC);

	switch (PieceType)
	{
	case EJanggiPieceType::Soldier:
	{
		const int32 Forward = Team == EJanggiTeam::Blue ? 1 : -1;
		if ((DR == Forward && DC == 0) || (DR == 0 && AC == 1)) return true;
		return DR == Forward && AC == 1 && IsConnectedPalaceDiagonal(FR, FC, TR, TC);
	}
	case EJanggiPieceType::Chariot:
		if ((FR == TR || FC == TC) && IsPathClearOrthogonal(FR, FC, TR, TC)) return true;
		if (AR == AC && IsConnectedPalaceDiagonal(FR, FC, TR, TC))
		{
			const int32 StepR = FMath::Sign(DR), StepC = FMath::Sign(DC);
			for (int32 R = FR + StepR, C = FC + StepC; R != TR; R += StepR, C += StepC)
				if (GetPieceAt(BoardIndexOf(R, C))) return false;
			return true;
		}
		return false;
	case EJanggiPieceType::Cannon:
	{
		if (FR != TR && FC != TC) return false;
		const int32 StepR = FMath::Sign(DR), StepC = FMath::Sign(DC);
		int32 Screens = 0;
		for (int32 R = FR + StepR, C = FC + StepC; R != TR || C != TC; R += StepR, C += StepC)
		{
			if (AActor* Screen = GetPieceAt(BoardIndexOf(R, C)))
			{
				if (GetPieceType(Screen) == EJanggiPieceType::Cannon) return false;
				++Screens;
			}
		}
		return Screens == 1 && (!DestinationPiece || GetPieceType(DestinationPiece) != EJanggiPieceType::Cannon);
	}
	case EJanggiPieceType::Horse:
		if (AR == 2 && AC == 1)
			return !GetPieceAt(BoardIndexOf(FR + FMath::Sign(DR), FC));
		if (AR == 1 && AC == 2)
			return !GetPieceAt(BoardIndexOf(FR, FC + FMath::Sign(DC)));
		return false;
	case EJanggiPieceType::Elephant:
		if (!((AR == 3 && AC == 2) || (AR == 2 && AC == 3))) return false;
		if (AR == 3)
		{
			const int32 SR = FMath::Sign(DR), SC = FMath::Sign(DC);
			return !GetPieceAt(BoardIndexOf(FR + SR, FC)) && !GetPieceAt(BoardIndexOf(FR + 2 * SR, FC + SC));
		}
		else
		{
			const int32 SR = FMath::Sign(DR), SC = FMath::Sign(DC);
			return !GetPieceAt(BoardIndexOf(FR, FC + SC)) && !GetPieceAt(BoardIndexOf(FR + SR, FC + 2 * SC));
		}
	case EJanggiPieceType::General:
	case EJanggiPieceType::Guard:
		return IsPalaceCell(FR, FC, Team) && IsPalaceCell(TR, TC, Team) &&
			((AR + AC == 1) || (AR == 1 && AC == 1 && IsConnectedPalaceDiagonal(FR, FC, TR, TC)));
	default:
		return false;
	}
}

bool AAuthoritativeJanggiBoard::IsPathClearOrthogonal(
	int32 FromRow, int32 FromColumn, int32 ToRow, int32 ToColumn) const
{
	const int32 StepR = FMath::Sign(ToRow - FromRow);
	const int32 StepC = FMath::Sign(ToColumn - FromColumn);
	for (int32 R = FromRow + StepR, C = FromColumn + StepC; R != ToRow || C != ToColumn; R += StepR, C += StepC)
		if (GetPieceAt(BoardIndexOf(R, C))) return false;
	return true;
}

bool AAuthoritativeJanggiBoard::IsPalaceCell(int32 Row, int32 Column, EJanggiTeam Team) const
{
	const bool CorrectRows = Team == EJanggiTeam::Blue ? Row >= 0 && Row <= 2 : Row >= 7 && Row <= 9;
	return CorrectRows && Column >= 3 && Column <= 5;
}

bool AAuthoritativeJanggiBoard::IsConnectedPalaceDiagonal(
	int32 FromRow, int32 FromColumn, int32 ToRow, int32 ToColumn) const
{
	if (FMath::Abs(ToRow - FromRow) != FMath::Abs(ToColumn - FromColumn)) return false;
	const int32 PalaceBase = FromRow <= 2 ? 0 : FromRow >= 7 ? 7 : INDEX_NONE;
	if (PalaceBase == INDEX_NONE || ToRow < PalaceBase || ToRow > PalaceBase + 2 ||
		FromColumn < 3 || FromColumn > 5 || ToColumn < 3 || ToColumn > 5) return false;
	const bool MainDiagonal = FromRow - PalaceBase == FromColumn - 3 && ToRow - PalaceBase == ToColumn - 3;
	const bool OtherDiagonal = FromRow - PalaceBase + FromColumn - 3 == 2 && ToRow - PalaceBase + ToColumn - 3 == 2;
	return MainDiagonal || OtherDiagonal;
}

void AAuthoritativeJanggiBoard::ClearExistingBlueprintSelection()
{
	if (FIntProperty* LastClick = FindPropertyNormalized<FIntProperty>(GetClass(), TEXT("LastClick")))
		LastClick->SetPropertyValue_InContainer(this, INDEX_NONE);
}

bool AAuthoritativeJanggiBoard::RemoveBoardPieceAt(int32 BoardIndex)
{
	if (!HasAuthority() || BoardIndex < 0 || BoardIndex >= BoardRows * BoardColumns) return false;
	AActor* Piece = GetPieceAt(BoardIndex);
	if (!IsValid(Piece)) return false;

	if (LastMovedPiece.Get() == Piece)
	{
		LastMovedPiece.Reset();
		ConsecutiveMovesForPiece = 0;
	}
	if (PieceCache.IsValidIndex(BoardIndex)) PieceCache[BoardIndex].Reset();

	FArrayProperty* ArrayProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("PieceMesh"));
	FObjectPropertyBase* ObjectProperty = ArrayProperty ? CastField<FObjectPropertyBase>(ArrayProperty->Inner) : nullptr;
	if (ArrayProperty && ObjectProperty)
	{
		void* Address = ArrayProperty->ContainerPtrToValuePtr<void>(this);
		FScriptArrayHelper Array(ArrayProperty, Address);
		if (Array.IsValidIndex(BoardIndex))
		{
			ObjectProperty->SetObjectPropertyValue(Array.GetRawPtr(BoardIndex), nullptr);
		}
	}

	const FString PieceName = Piece->GetName();
	const bool bDestroyAccepted = Piece->Destroy(true);
	UE_LOG(LogTemp, Display, TEXT("BOARD_PIECE_REMOVED index=%d actor=%s destroy_accepted=%s"),
		BoardIndex, *PieceName, bDestroyAccepted ? TEXT("true") : TEXT("false"));
	return bDestroyAccepted;
}

void AAuthoritativeJanggiBoard::CompleteBoardMove(int32 DestinationIndex)
{
	AActor* MovingPiece = GetPieceAt(SelectedIndex);
	AActor* CapturedPiece = GetPieceAt(DestinationIndex);
	if (!MovingPiece) return;
	if (CapturedPiece && CapturedPiece != MovingPiece)
	{
		const EJanggiTeam CapturedTeam = GetPieceTeam(CapturedPiece);
		const bool bCapturedRemoved = RemoveBoardPieceAt(DestinationIndex);
		UE_LOG(LogTemp, Display, TEXT("BOARD_BATTLE_LOSER_REMOVED team=%s index=%d removed=%s role=defender"),
			*UEnum::GetValueAsString(CapturedTeam), DestinationIndex,
			bCapturedRemoved ? TEXT("true") : TEXT("false"));
	}
	if (PieceCache.Num() == BoardRows * BoardColumns)
	{
		PieceCache[DestinationIndex] = MovingPiece;
		PieceCache[SelectedIndex].Reset();
	}
	FArrayProperty* ArrayProperty = FindPropertyNormalized<FArrayProperty>(GetClass(), TEXT("PieceMesh"));
	FObjectPropertyBase* ObjectProperty = ArrayProperty ? CastField<FObjectPropertyBase>(ArrayProperty->Inner) : nullptr;
	if (ArrayProperty && ObjectProperty)
	{
		void* Address = ArrayProperty->ContainerPtrToValuePtr<void>(this);
		FScriptArrayHelper Array(ArrayProperty, Address);
		if (Array.Num() < BoardRows * BoardColumns) Array.Resize(BoardRows * BoardColumns);
		ObjectProperty->SetObjectPropertyValue(Array.GetRawPtr(SelectedIndex), nullptr);
		ObjectProperty->SetObjectPropertyValue(Array.GetRawPtr(DestinationIndex), MovingPiece);
	}
	MovingPiece->SetActorLocation(
		GetActorTransform().TransformPosition(GetCellLocalPosition(DestinationIndex)), false, nullptr, ETeleportType::TeleportPhysics);
	ClickHistory.Add(FVector_NetQuantize10(GetCellLocalPosition(DestinationIndex)));
	if (LastMovedPiece.Get() == MovingPiece) ++ConsecutiveMovesForPiece;
	else { LastMovedPiece = MovingPiece; ConsecutiveMovesForPiece = 1; }
	SelectedIndex = INDEX_NONE;
	ClearLegalMoveMarkersForAllPlayers();
	ForceNetUpdate();
}

void AAuthoritativeJanggiBoard::StartTurn(EJanggiTeam Team)
{
	ClearLegalMoveMarkersForAllPlayers();
	CurrentTurnTeam = Team;
	MatchPhase = EBoardMatchPhase::BoardTurn;
	if (SelectedIndex != INDEX_NONE)
	{
		ClearExistingBlueprintSelection();
	}
	SelectedIndex = INDEX_NONE;
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	TurnDeadlineServerTime = GetWorld()->GetTimeSeconds() + TurnTimeLimitSeconds;
	GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &AAuthoritativeJanggiBoard::HandleTurnExpired, TurnTimeLimitSeconds, false);
	ForceNetUpdate();
}

void AAuthoritativeJanggiBoard::HandleTurnExpired()
{
	if (!HasAuthority() || MatchPhase != EBoardMatchPhase::BoardTurn) return;
	ClearExistingBlueprintSelection();
	StartTurn(CurrentTurnTeam == EJanggiTeam::Blue ? EJanggiTeam::Red : EJanggiTeam::Blue);
}

void AAuthoritativeJanggiBoard::StartArenaBattle(int32 DestinationIndex)
{
	AActor* Attacker = GetPieceAt(SelectedIndex);
	AActor* Defender = GetPieceAt(DestinationIndex);
	if (!Attacker || !Defender) return;
	ClearLegalMoveMarkersForAllPlayers();
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	bBoardInputPaused = true;
	MatchPhase = EBoardMatchPhase::ArenaTransition;
	BattleContext.bActive = true;
	BattleContext.OriginIndex = SelectedIndex;
	BattleContext.DestinationIndex = DestinationIndex;
	BattleContext.AttackerTeam = CurrentTurnTeam;
	BattleContext.AttackerType = GetPieceType(Attacker);
	BattleContext.DefenderType = GetPieceType(Defender);
	BattleContext.Attacker = Attacker;
	BattleContext.Defender = Defender;
	SpawnArenaPlaceholderCharacters();
	OnArenaBattleStarted(BattleContext);
	UE_LOG(LogTemp, Display, TEXT("BOARD_BATTLE_STARTED attacker=%s from=%d to=%d"),
		*UEnum::GetValueAsString(BattleContext.AttackerTeam), BattleContext.OriginIndex, BattleContext.DestinationIndex);
	NotifyArenaCamera(true);
	GetWorldTimerManager().SetTimer(ArenaTransitionTimerHandle, this, &AAuthoritativeJanggiBoard::EnterArenaBattlePhase, ArenaTransitionSeconds, false);
	ForceNetUpdate();
}

void AAuthoritativeJanggiBoard::EnterArenaBattlePhase()
{
	if (!HasAuthority() || !BattleContext.bActive) return;
	MatchPhase = EBoardMatchPhase::ArenaBattle;
	GetWorldTimerManager().SetTimer(ArenaBattleTimerHandle, this, &AAuthoritativeJanggiBoard::HandleArenaBattleExpired, ArenaBattleTimeLimitSeconds, false);
	ForceNetUpdate();
}

void AAuthoritativeJanggiBoard::HandleArenaBattleExpired()
{
	const EJanggiTeam DefenderTeam = BattleContext.AttackerTeam == EJanggiTeam::Blue ? EJanggiTeam::Red : EJanggiTeam::Blue;
	ResolveArenaBattle(DefenderTeam);
}

void AAuthoritativeJanggiBoard::NotifyArenaCamera(bool bEnteringArena)
{
	const FVector BlueFighterLocation = GetArenaFighterWorldLocation(EJanggiTeam::Blue);
	const FVector RedFighterLocation = GetArenaFighterWorldLocation(EJanggiTeam::Red);
	const FVector BlueCameraLocation = GetArenaCameraWorldLocation(EJanggiTeam::Blue);
	const FVector RedCameraLocation = GetArenaCameraWorldLocation(EJanggiTeam::Red);
	const FVector FocusLocation = GetArenaCameraFocusWorldLocation();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get()))
		{
			if (bEnteringArena) Controller->ClientBeginArenaTransition(
				FocusLocation,
				BlueFighterLocation,
				RedFighterLocation,
				BlueCameraLocation,
				RedCameraLocation,
				ArenaTransitionSeconds);
			else Controller->ClientEndArenaTransition(ArenaTransitionSeconds);
		}
	}
}

FVector AAuthoritativeJanggiBoard::GetArenaCenterWorldLocation() const
{
	return (GetArenaFighterWorldLocation(EJanggiTeam::Blue) + GetArenaFighterWorldLocation(EJanggiTeam::Red)) * 0.5f;
}

FVector AAuthoritativeJanggiBoard::GetArenaFighterWorldLocation(EJanggiTeam Team) const
{
	return GetActorTransform().TransformPosition(
		Team == EJanggiTeam::Red ? ArenaRedSpawnOffset : ArenaBlueSpawnOffset);
}

FVector AAuthoritativeJanggiBoard::GetArenaCameraWorldLocation(EJanggiTeam Team) const
{
	return GetActorTransform().TransformPosition(
		Team == EJanggiTeam::Red ? ArenaRedCameraOffset : ArenaBlueCameraOffset);
}

FVector AAuthoritativeJanggiBoard::GetArenaCameraFocusWorldLocation() const
{
	return GetActorTransform().TransformPosition(ArenaCameraFocusOffset);
}

namespace
{
const TCHAR* GetArenaCharacterMeshPath(EJanggiTeam Team, EJanggiPieceType Type)
{
	const bool bBlue = Team == EJanggiTeam::Blue;
	switch (Type)
	{
	case EJanggiPieceType::Soldier: return bBlue
		? TEXT("/Game/User/Charactor/Soldier_B/SK_Soldier_B_CharacterWeapon.SK_Soldier_B_CharacterWeapon")
		: TEXT("/Game/User/Charactor/Soldier_R/SK_Soldier_R_CharacterWeapon.SK_Soldier_R_CharacterWeapon");
	case EJanggiPieceType::Cannon: return bBlue
		? TEXT("/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeapon.SK_Cannon_B_CharacterWeapon")
		: TEXT("/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeapon.SK_Cannon_R_CharacterWeapon");
	case EJanggiPieceType::Chariot: return bBlue
		? TEXT("/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeapon.SK_Chariot_B_CharacterWeapon")
		: TEXT("/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeapon.SK_Chariot_R_CharacterWeapon");
	case EJanggiPieceType::Elephant: return bBlue
		? TEXT("/Game/User/Charactor/Elephant_B/SK_Elephant_B_CharacterWeapon.SK_Elephant_B_CharacterWeapon")
		: TEXT("/Game/User/Charactor/Elephant_R/SK_Elephant_R_CharacterWeapon.SK_Elephant_R_CharacterWeapon");
	case EJanggiPieceType::Horse: return bBlue
		? TEXT("/Game/User/Asset/Horse_blue_.Horse_blue_")
		: TEXT("/Game/User/Charactor/Horse_R/SK_Horse_R_CharacterWeapon.SK_Horse_R_CharacterWeapon");
	case EJanggiPieceType::Guard: return bBlue
		? TEXT("/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeapon.SK_Guard_B_CharacterWeapon")
		: TEXT("/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeapon.SK_Guard_R_CharacterWeapon");
	case EJanggiPieceType::General: return bBlue
		? TEXT("/Game/User/Charactor/King_B/SK_King_B_CharacterWeapon.SK_King_B_CharacterWeapon")
		: TEXT("/Game/User/Charactor/King_R/SK_King_R_CharacterWeapon.SK_King_R_CharacterWeapon");
	default: return nullptr;
	}
}
}

void AAuthoritativeJanggiBoard::SpawnArenaPlaceholderCharacters()
{
	if (!HasAuthority() || !GetWorld() || !BattleContext.bActive) return;
	DestroyArenaPlaceholderCharacters();
	const EJanggiTeam DefenderTeam = BattleContext.AttackerTeam == EJanggiTeam::Blue
		? EJanggiTeam::Red : EJanggiTeam::Blue;

	auto SpawnCharacter = [this](EJanggiTeam Team, EJanggiPieceType Type) -> AArenaPlaceholderCharacter*
	{
		const TCHAR* MeshPath = GetArenaCharacterMeshPath(Team, Type);
		USkeletalMesh* Mesh = MeshPath ? LoadObject<USkeletalMesh>(nullptr, MeshPath) : nullptr;
		if (!Mesh)
		{
			UE_LOG(LogTemp, Error, TEXT("BOARD_ARENA_CHARACTER_MESH_FAILED team=%s type=%s"),
				*UEnum::GetValueAsString(Team), *UEnum::GetValueAsString(Type));
			return nullptr;
		}
		const FVector Location = GetArenaFighterWorldLocation(Team);
		const FVector OpponentLocation = GetArenaFighterWorldLocation(Team == EJanggiTeam::Blue ? EJanggiTeam::Red : EJanggiTeam::Blue);
		FTransform Transform((OpponentLocation - Location).Rotation(), Location, FVector(ArenaPlaceholderScale));
		AArenaPlaceholderCharacter* Character = GetWorld()->SpawnActorDeferred<AArenaPlaceholderCharacter>(
			AArenaPlaceholderCharacter::StaticClass(), Transform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Character) return nullptr;
		Character->Tags.Add(Team == EJanggiTeam::Blue ? FName(TEXT("ArenaBlue")) : FName(TEXT("ArenaRed")));
		Character->SetCharacterMesh(Mesh);
		UGameplayStatics::FinishSpawningActor(Character, Transform);
		return Character;
	};

	if (BattleContext.AttackerTeam == EJanggiTeam::Blue)
	{
		ArenaBlueCharacter = SpawnCharacter(EJanggiTeam::Blue, BattleContext.AttackerType);
		ArenaRedCharacter = SpawnCharacter(EJanggiTeam::Red, BattleContext.DefenderType);
	}
	else
	{
		ArenaRedCharacter = SpawnCharacter(EJanggiTeam::Red, BattleContext.AttackerType);
		ArenaBlueCharacter = SpawnCharacter(EJanggiTeam::Blue, BattleContext.DefenderType);
	}
	UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_CHARACTERS_SPAWNED center=%s blue=%s red=%s"),
		*GetArenaCenterWorldLocation().ToCompactString(),
		*GetArenaFighterWorldLocation(EJanggiTeam::Blue).ToCompactString(),
		*GetArenaFighterWorldLocation(EJanggiTeam::Red).ToCompactString());
}

void AAuthoritativeJanggiBoard::DestroyArenaPlaceholderCharacters()
{
	if (ArenaBlueCharacter.IsValid()) ArenaBlueCharacter->Destroy();
	if (ArenaRedCharacter.IsValid()) ArenaRedCharacter->Destroy();
	ArenaBlueCharacter.Reset();
	ArenaRedCharacter.Reset();
}

bool AAuthoritativeJanggiBoard::FindFirstLegalNonCaptureMove(
	EJanggiTeam Team, int32& OutFromIndex, int32& OutToIndex) const
{
	RebuildPieceCache();
	int32 OccupiedCount = 0;
	int32 TeamPieceCount = 0;
	for (int32 FromIndex = 0; FromIndex < BoardRows * BoardColumns; ++FromIndex)
	{
		AActor* Piece = GetPieceAt(FromIndex);
		if (Piece) ++OccupiedCount;
		if (!Piece || GetPieceTeam(Piece) != Team) continue;
		++TeamPieceCount;
		const EJanggiPieceType Type = GetPieceType(Piece);
		for (int32 To = 0; To < BoardRows * BoardColumns; ++To)
		{
			if (!GetPieceAt(To) && IsLegalMove(FromIndex, To, Type, Team))
			{
				OutFromIndex = FromIndex;
				OutToIndex = To;
				return true;
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("BOARD_OCCUPANCY_DIAGNOSTIC occupied=%d team_pieces=%d team=%s"),
		OccupiedCount, TeamPieceCount, *UEnum::GetValueAsString(Team));
	for (TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(*It))
		{
			const void* Address = ArrayProperty->ContainerPtrToValuePtr<void>(this);
			FScriptArrayHelper Array(ArrayProperty, Address);
			UE_LOG(LogTemp, Error, TEXT("BOARD_ARRAY_DIAGNOSTIC name=%s inner=%s count=%d"),
				*ArrayProperty->GetName(), *ArrayProperty->Inner->GetClass()->GetName(), Array.Num());
		}
	}
	return false;
}
