param(
    [Parameter(Mandatory=$true)][string]$Target,
    [Parameter(Mandatory=$true)][string]$Injector
)

$ErrorActionPreference = "Stop"
$logPath = Join-Path ([System.IO.Path]::GetTempPath()) ("wininj-poc01-" + [guid]::NewGuid().ToString("N") + ".log")
$targetProcess = Start-Process -FilePath $Target -ArgumentList @("--duration-sec", "10") -PassThru -RedirectStandardOutput $logPath

try {
    $injectorProcess = Start-Process -FilePath $Injector -ArgumentList @("$($targetProcess.Id)") -PassThru -Wait
    if ($null -ne $injectorProcess.ExitCode -and $injectorProcess.ExitCode -ne 0) {
        throw "Controller exited unsuccessfully"
    }

    if (-not $targetProcess.WaitForExit(11000)) {
        throw "Target timed out"
    }

    $output = [System.IO.File]::ReadAllText($logPath)
    if ($output -notmatch "payload_marker=POC01_PAYLOAD_INSIDE_TARGET" -or $output -notmatch "payload_unloaded=true") {
        throw "Target did not report successful payload execution: $output"
    }

    if ($null -ne $targetProcess.ExitCode -and $targetProcess.ExitCode -ne 0) {
        throw "Target returned a nonzero exit code"
    }

    Write-Host "poc01-cooperative-load passed"
} finally {
    if (-not $targetProcess.HasExited) {
        $targetProcess.Kill()
        $targetProcess.WaitForExit()
    }
    Remove-Item -LiteralPath $logPath -Force -ErrorAction SilentlyContinue
}
