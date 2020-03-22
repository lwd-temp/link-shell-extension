$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2926/ln.zip'
$checksum = '0d169b19d66bde36f09e2209f6044f3972ca884df8aa54e8f0f227d338515d47'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2926/ln64.zip'
$checksum64 = 'cd09d37e14ab52c594e74bfe1b7044dad25af15c56db04a7b4c505100cdb8d08'
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