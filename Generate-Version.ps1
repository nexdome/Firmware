# Uses GitVersion command line tools to create a Version.h header file.
param (
    [string]$OutputFile = ".\Version.h",
	[string]$ProjectDirectory = "."
)

Push-Location $ProjectDirectory

$gitVersion = GitVersion.exe | ConvertFrom-Json
$semver = $gitVersion.SemVer
$majorVersion = $gitVersion.Major
$minorVersion = $gitVersion.Minor

Set-Content -Path $OutputFile "#define SemanticVersion ""$semver"""
Add-Content -Path $OutputFile "#define MajorVersion ($majorVersion)"
Add-Content -Path $OutputFile "#define MinorVersion ($minorVersion)"
Write-Debug "Wrote semantic version: $semver"
Pop-Location