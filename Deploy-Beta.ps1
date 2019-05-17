# Uses GitVersion command line tools to create a Version.h header file.
param (
    [string]$OutputFile = ".\Version.h",
    [string]$ProjectDirectory = ".",
    [string]$DeployDirectory = "F:\OneDrive\Clients\NexDome (Babak Sedehi)\NexDome Beta\"
)

Push-Location $ProjectDirectory

$gitVersion = GitVersion.exe | ConvertFrom-Json
$semver = $gitVersion.SemVer
$majorVersion = $gitVersion.Major
$minorVersion = $gitVersion.Minor

if (-not ($semver -Match "-beta")) {
    throw "The working copy has semantic version $semver, which is not a prerelease build"
}

$betaPath = [System.IO.Path]::Combine($DeployDirectory, $semver)
$uploaderPath = [System.IO.Path]::Combine($DeployDirectory, "Uploader")
$uploaderPublish = "C:\Users\Tim\source\repos\TA.NexDome\TA.NexDome.FirmwareUpdater\bin\Release\netcoreapp2.1\publish\"

if (-not (Test-Path -LiteralPath $betaPath)) {
    
    try {
        New-Item -Path $betaPath -ItemType Directory -ErrorAction Stop | Out-Null #-Force
    }
    catch {
        Write-Error -Message "Unable to create directory '$DirectoryToCreate'. Error was: $_" -ErrorAction Stop
    }
}
else {
    Remove-Item "$betaPath\*" -Recurse -Force
}

Copy-Item ".\TA.NexDome.Rotator\Release\TA.NexDome.Rotator.hex" -Destination "$betaPath\" -Force
Copy-Item ".\TA.NexDome.Shutter\Release\TA.NexDome.Shutter.hex" -Destination "$betaPath\" -Force
Copy-Item ".\XBeeFactoryReset\Release\XBeeFactoryReset.hex" -Destination "$betaPath\" -Force
Copy-Item ".\ReadMe.pdf" -Destination "$betaPath\" -Force
Copy-Item "$uploaderPublish" -Destination "$uploaderPath" -Recurse -Force

Pop-Location
