[CmdletBinding()]
Param(
    [Parameter(Mandatory = $True, Position = 1)]
    [string]$comPort,
    [Parameter(Mandatory = $True, Position = 2)]
    [string]$hexFile
)

$avrdude = ".\avrdude.exe"
$avrdudeConfig = ".\avrdude.conf"
$bootloaderPort = RunLeonardoBootloader($comPort)
$arguments = "-C$avrdudeConfig", "-v", "-patmega32u4", "-cavr109", "-b57600", "-P$comPort", "-D", "-Uflash:w:$hexFile`:i"
& $avrdude $arguments

function RunLeonardoBootloader {
    param (
        $nonBootloaderComPort
    )

    $availablePorts = [System.IO.Ports.SerialPort]::GetPortNames()
    if (NOT $availablePorts -contains $nonBootloaderComPort)
        {
            Write-Error -Message "Invalid COM port name specified"
            Exit
        }

    $port=New-Object System.IO.Ports.SerialPort
    $port.BaudRate=1200
    $port.PortName=$nonBootloaderComPort
    $port.Open()
    $port.Close()
    <#
    The Arduino Leonardo will now enter bootloader mode and will wait for up to 8 seconds for a firmware upload.
    The bootloader will appear on a new COM port and we have to detect that port.
    #>

    
    */  
}
