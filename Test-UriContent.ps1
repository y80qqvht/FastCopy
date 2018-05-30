<#
.SYNOPSIS
指定された URL の内容が更新されたかどうかをチェックする。

.PARAMETER Uri
対象の URL 文字列。

.PARAMETER HeadersFilePath
レスポンスヘッダーを保存するファイルのパス（ワイルドカードを展開しない）。

.INPUTS
なし。

.OUTPUTS
System.Net.HttpStatusCode。
内容が更新されていた場合は OK。
内容が更新されていない場合は NotModified。
#>
[CmdletBinding()]
Param (
    [Parameter(Position = 0)]
    [ValidateNotNullOrEmpty()]
    [System.String] $Uri = 'https://fastcopy.jp/',

    [Parameter(Position = 1)]
    [ValidateNotNullOrEmpty()]
    [System.String] $HeadersFilePath =
        [System.IO.Path]::ChangeExtension($MyInvocation.MyCommand.Path, '.xml')
)

[System.Net.HttpWebRequest] $req =
    [System.Net.WebRequest]::Create($Uri) -as [System.Net.HttpWebRequest]

$req.AllowAutoRedirect = $false
$req.Timeout = 10000
$req.Method = 'HEAD'
$req.Accept = 'text/html'
$req.Headers.Set([System.Net.HttpRequestHeader]::AcceptLanguage, 'ja')

if (Test-Path -LiteralPath $HeadersFilePath -PathType Leaf) {
    [System.Management.Automation.PSObject] $settings =
        Import-Clixml -Path $HeadersFilePath
    if ($settings.'ETag') {
        $req.Headers.Set([System.Net.HttpRequestHeader]::IfNoneMatch, `
            $settings.'ETag')
    } elseif ($settings.'Last-Modified') {
        $req.IfModifiedSince = Get-Date $settings.'Last-Modified'
    }
}

[System.Net.HttpWebResponse] $res = $null
try {
    $res = $req.GetResponse() -as [System.Net.HttpWebResponse]
    if ($res.StatusCode -eq [System.Net.HttpStatusCode]::OK) {
        [System.Collections.Hashtable] $props = @{}
        $res.Headers.AllKeys | ForEach-Object {
            $props.Add($_, $res.Headers[$_])
        }

        New-Object 'System.Management.Automation.PSObject' -Property $props |
            Export-Clixml -Path $HeadersFilePath -Encoding UTF8
    }
    $res.StatusCode
} catch [System.Net.WebException] {
    [System.Net.WebException] $ex = $_.Exception
    if ($ex.Status -eq [System.Net.WebExceptionStatus]::ProtocolError) {
        $res = $ex.Response -as [System.Net.HttpWebResponse]
        $res.StatusCode
    } else {
        throw
    }
} finally {
    if ($res -ne $null) {
        $res.Close()
    }
}
