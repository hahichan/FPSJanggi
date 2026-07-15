param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [ValidateRange(5, 120)]
    [int]$DurationSeconds = 15
)

$ErrorActionPreference = 'Stop'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$project = Join-Path $ProjectRoot 'FPSJanggi.uproject'
$logPath = Join-Path $ProjectRoot 'Saved\Logs\StandaloneLobbySmoke.log'

if (-not (Test-Path -LiteralPath $editor)) { throw "UnrealEditor-Cmd.exe not found: $editor" }
if (-not (Test-Path -LiteralPath $project)) { throw "Project not found: $project" }

$process = $null
try {
    $arguments = @($project, '/Game/User/Map/MainMap', '-game', '-log=StandaloneLobbySmoke.log', '-unattended', '-nullrhi', '-nosound', '-nosplash', '-nosteam')
    $process = Start-Process -FilePath $editor -ArgumentList $arguments -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds $DurationSeconds
}
finally {
    if ($process -and -not $process.HasExited) { Stop-Process -Id $process.Id }
}

if (-not (Test-Path -LiteralPath $logPath)) { throw "Lobby smoke log not created: $logPath" }
$log = Get-Content -LiteralPath $logPath
$required = @('LOBBY_UI_READY mode=standalone', 'LOBBY_CAMERA_READY')
foreach ($pattern in $required) {
    if (-not ($log | Select-String -SimpleMatch $pattern)) {
        throw "Lobby smoke missing required log: $pattern"
    }
}

$failures = $log | Select-String -Pattern @('Fatal error:', 'ensure condition failed', 'BOARD_CAMERA_FAILED')
if ($failures) {
    $failures | ForEach-Object { Write-Output $_.Line }
    throw 'Lobby smoke reported a fatal or camera failure.'
}

$log | Select-String -Pattern @('LOBBY_UI_READY', 'LOBBY_CAMERA_READY') | ForEach-Object { Write-Output $_.Line }
Write-Output 'STANDALONE_LOBBY_SMOKE passed=true'
