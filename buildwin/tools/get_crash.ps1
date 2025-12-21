$events = Get-WinEvent -LogName 'Application' -MaxEvents 200 | Where-Object { $_.Id -eq 1000 }
if ($events) {
    $events | Select-Object -First 5 | ForEach-Object {
        Write-Host "===================="
        Write-Host "Time: $($_.TimeCreated)"
        Write-Host $_.Message
        Write-Host ""
    }
} else {
    Write-Host "No crash events found"
}
