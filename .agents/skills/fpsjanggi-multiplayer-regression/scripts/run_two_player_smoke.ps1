param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [int]$Port = 18889,
    [ValidateRange(5, 120)]
    [int]$DurationSeconds = 20,
    [switch]$Arena,
    [switch]$General
)

$ErrorActionPreference = 'Stop'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$project = Join-Path $ProjectRoot 'FPSJanggi.uproject'
$logDir = Join-Path $ProjectRoot 'Saved\Logs'
$serverLogName = 'MultiplayerSmokeServer.log'
$clientLogName = 'MultiplayerSmokeClient.log'
$testFlag = if ($General) { '-GeneralSmokeTest' } elseif ($Arena) { '-ArenaSmokeTest' } else { '-BoardSmokeTest' }

if (-not (Test-Path -LiteralPath $editor)) { throw "UnrealEditor-Cmd.exe not found: $editor" }
if (-not (Test-Path -LiteralPath $project)) { throw "Project not found: $project" }

$serverArgs = @($project, '/Game/User/Map/MainMap?listen', '-game', "-port=$Port", "-log=$serverLogName", '-unattended', '-nullrhi', '-nosound', '-nosplash', '-nosteam', $testFlag)
$clientArgs = @($project, "127.0.0.1:$Port", '-game', "-log=$clientLogName", '-unattended', '-nullrhi', '-nosound', '-nosplash', '-nosteam')

$server = $null
$client = $null
try {
    $server = Start-Process -FilePath $editor -ArgumentList $serverArgs -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds 5
    $client = Start-Process -FilePath $editor -ArgumentList $clientArgs -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds $DurationSeconds
}
finally {
    if ($client -and -not $client.HasExited) { Stop-Process -Id $client.Id }
    if ($server -and -not $server.HasExited) { Stop-Process -Id $server.Id }
}

$patterns = @('BOARD_CAMERA_READY', 'BOARD_CAMERA_FAILED', 'BOARD_INITIALIZATION', 'BOARD_MATCH_STARTED', 'BOARD_SMOKE_TEST', 'BOARD_GENERAL_SMOKE_TEST', 'BOARD_MATCH_FINISHED', 'BOARD_ARENA_SMOKE_TEST', 'BOARD_ARENA_CHARACTERS_SPAWNED', 'BOARD_ARENA_PLACEHOLDER_READY', 'BOARD_ARENA_PLACEHOLDER_MESH', 'BOARD_ARENA_CAMERA_READY', 'BOARD_ARENA_CAMERA_RETURN', 'BOARD_ARENA_DEBUG_UI_READY', 'BOARD_ARENA_DEBUG_WINNER_REQUEST', 'BOARD_BATTLE_RESOLVED', 'BOARD_PIECE_REMOVED', 'BOARD_BATTLE_LOSER_REMOVED', 'BOARD_STATE', 'BOARD_VISUALS_READY', 'BOARD_STATUS_UI_BUILT', 'BOARD_STATUS_UI_READY', 'BOARD_LEGAL_MOVE_OVERLAY', 'BOARD_LEGAL_MOVE_OVERLAY_PROJECTED', 'BOARD_LEGAL_MOVES_SHOWN', 'BOARD_LEGAL_MOVE_WORLD_MARKER_FAILED')
foreach ($name in @($serverLogName, $clientLogName)) {
    $path = Join-Path $logDir $name
    if (-not (Test-Path -LiteralPath $path)) { throw "Smoke log not created: $path" }
    Write-Output "[$name]"
    Select-String -LiteralPath $path -Pattern $patterns | ForEach-Object { $_.Line }
}
