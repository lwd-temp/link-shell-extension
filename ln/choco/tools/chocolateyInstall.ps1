$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2932/ln.zip'
$checksum = 'e0483b86c627f05f6fbb3fb9f122b7e8a82d1c38352f1e6bf09b65611246e64a'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2932/ln64.zip'
$checksum64 = '109c4afba7320a580f8c39f811bfaa081e6f40e3f746bc99ba511338ee092b8e'
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