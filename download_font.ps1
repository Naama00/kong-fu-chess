# יצירת תיקיית assets במידה והיא אינה קיימת
if (-not (Test-Path -Path "assets")) {
    New-Item -ItemType Directory -Path "assets"
    Write-Host "Created assets directory."
}

# כתובת ישירה לשרת הקבצים הגולמיים של GitHub (raw.githubusercontent.com)
$fontUrl = "https://raw.githubusercontent.com/googlefonts/roboto/main/src/hinted/Roboto-Regular.ttf"
$outputPath = "assets/arial.ttf"

Write-Host "Downloading font from raw server..."
try {
    # הגדרת אבטחה לחיבור TLS 1.2 ומעלה, ושימוש בבקרת הורדה ישירה
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    
    # הורדת הקובץ הבינארי
    Invoke-WebRequest -Uri $fontUrl -OutFile $outputPath -UseBasicParsing
    
    Write-Host "Font downloaded successfully to: $outputPath" -ForegroundColor Green
} catch {
    Write-Error "Failed to download font: $_"
}