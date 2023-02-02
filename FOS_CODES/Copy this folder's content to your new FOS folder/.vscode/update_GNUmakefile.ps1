$curFile = "$(Get-Location)\GNUmakefile"
$check = Get-Content $curFile -tail 1
if ($check -eq "#QEMU added by Mohamed Samy") {
    exit
}
$update = "QEMU := qemu-system-i386 `n"
$update = $update + "GDBPORT 	= 26000 `n"
$update = $update + "QEMUGDB 	= -gdb tcp::`$(GDBPORT) `n"
$update = $update + "QEMUOPTS 	= -drive file=`$(IMAGES),media=disk,format=raw -smp 2 -m 32 `$(QEMUEXTRAS)  `n"
$update = $update + "qemu: all `n"
$update = $update + "`t`$(QEMU) -serial mon:stdio `$(QEMUOPTS) `n"
$update = $update + "qemu-gdb: all `n"
$update = $update + "`t`$(QEMU) `$(QEMUOPTS) -S `$(QEMUGDB)  `n"
$update = $update + "#QEMU added by Mohamed Samy" #to check if the script has been run before
Add-Content -Path $curFile -Value $update
