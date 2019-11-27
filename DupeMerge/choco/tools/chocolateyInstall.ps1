$packageName = 'dupemerge'
$url = 'https://schinagl.priv.at/nt/dupemerge/save/1103/dupemerge.zip'
$checksum = '89dd7d2046038a0634b9e3bcc866103e05741b76d64c553dd3cbb17170a3e389'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/dupemerge/save/1103/dupemerge64.zip'
$checksum64 = '2237c2e9cc11a6aded52b79151b6e1b19866a7fb564f82dbed65c046afa7492e'
$checksumType64 = 'sha256'
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"

Install-ChocolateyZipPackage -PackageName "$packageName" `
                             -Url "$url" `
                             -UnzipLocation "$toolsDir" `
                             -Url64bit "$url64" `
                             -Checksum "$checksum" `
                             -ChecksumType "$checksumType" `
                             -Checksum64 "$checksum64" `
                             -ChecksumType64 "$checksumType64"