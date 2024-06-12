$input_text = usbipd wsl list
$target_port

for ($i = 0; $i -lt $input_text.Count; $i++) {
#    Write-Host $input_text[$i]

    if($input_text[$i].Contains("CH9102") )
    {
        Write-Host "----Find COM Port----"
        $port_array = [System.IO.Ports.SerialPort]::GetPortNames()
        for ($j = 0; $j -lt $port_array.Count; $j++) 
        {
            if($input_text[$i].Contains($port_array[$j]))
            {
                Write-Host "Target PORT : " $port_array[$j]
                $target_port = $port_array[$j]
            }
        }        
    }

}



$port = New-Object System.IO.Ports.SerialPort $target_port, 115200, None, 8, one

$port.open()

for($j=0; $j -lt 3; $j++)
{
    $port.WriteLine("Hello");
}


#$port.WriteLine("Hello");
#$port.WriteLine("Bye");

$port.Close()

# Pause # 여기서는 필요없음
