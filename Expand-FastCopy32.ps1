<#
.SYNOPSIS
指定されたパスのインストーラーから 32 ビットの実行ファイルを抽出する。

.PARAMETER Path
インストーラーのパス文字列。

.PARAMETER DestinationPath
抽出した実行ファイルの保存先のパス文字列。

.INPUTS
なし。

.OUTPUTS
なし。
#>
[CmdletBinding()]
Param (
    [Parameter(Position = 0)]
    [ValidateNotNullOrEmpty()]
    [System.String] $Path = '.\FastCopy*_installer.exe',

    [Parameter(Position = 1)]
    [ValidateNotNullOrEmpty()]
    [System.String] $DestinationPath = '.'
)

End {
    if (-not (Test-Path -LiteralPath $DestinationPath)) {
        New-Item -ErrorAction Stop -ItemType Directory -Path $DestinationPath | Out-Null
    }
    [System.String] $dstPath = Convert-Path -ErrorAction Stop -LiteralPath $DestinationPath
    Get-ChildItem -Path $Path -File | ForEach-Object -Process {
        [System.String] $exeName = $_.Name
        [System.String] $exePath = $_.FullName
        [System.String] $dirName = $exeName -creplace '_.+?\.exe$', '_32'
        [System.String] $dirPath = Join-Path -Path $dstPath -ChildPath $dirName
        if (Test-Path -LiteralPath $dirPath) {
            Write-Error -ErrorAction Stop -Message "Already exists: $dirPath"
        }
        Start-Process -Wait -FilePath $exePath -ArgumentList "/DIR=$dirPath", '/EXTRACT32', '/NOSUBDIR'
    }
}
