# Copies built firmware files into the ASCOm driver project where an installer can be built
param (
    [string]$ProjectDirectory = ".",
    [string]$DeployDirectory = "..\TA.NexDome.AscomServer\Firmware"
)

Write-Host "Deploying to $DeployDirectory"
Push-Location $ProjectDirectory

$gitVersion = GitVersion.exe | ConvertFrom-Json
$semver = $gitVersion.SemVer
$majorVersion = $gitVersion.Major
$minorVersion = $gitVersion.Minor


if (-not (Test-Path -LiteralPath $DeployDirectory)) {
    try {
        New-Item -Path $DeployDirectory -ItemType Directory -ErrorAction Stop | Out-Null -Force
    }
    catch {
        Write-Error -Message "Unable to create directory '$DeployDirectory'. Error was: $_" -ErrorAction Stop
    }
}
else {
    Remove-Item "$DeployDirectory\*.hex" -ErrorAction Ignore
    Remove-Item "$DeployDirectory\ReadMe.pdf" -ErrorAction Ignore
}

Copy-Item ".\TA.NexDome.Rotator\Release\TA.NexDome.Rotator.hex" -Destination "$DeployDirectory\Rotator-$semver.hex" -Force
Copy-Item ".\TA.NexDome.Shutter\Release\TA.NexDome.Shutter.hex" -Destination "$DeployDirectory\Shutter-$semver.hex" -Force

Write-Host "Deploy complete. Now build the ASCOM driver solution."

Pop-Location
