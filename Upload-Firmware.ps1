[CmdletBinding()]
Param(
    [Parameter(Mandatory = $True, Position = 1)]
    [string]$comPort,
    [Parameter(Mandatory = $True, Position = 2)]
    [string]$hexFile
)

$avrdude = "C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avrdude.exe"
$avrdudeConfig = "C:\Program Files (x86)\Arduino\hardware\tools\avr\etc\avrdude.conf"

$arguments = "-C$avrdudeConfig", "-v", "-patmega32u4", "-cavr109", "-b57600", "-P$comPort", "-D", "-Uflash:w:$hexFile`:i"

& $avrdude $arguments
