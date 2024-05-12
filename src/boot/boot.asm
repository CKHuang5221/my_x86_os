ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


jmp short start
nop

; FAT16 Header
OEMIdentifier           db 'PEACHOS '
BytesPerSector          dw 0x200
SectorsPerCluster       db 0x80
ReservedSectors         dw 200
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFat           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773594

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'PEACHOS BOO'
SystemIDString          db 'FAT16   '


start:
    jmp 0:step2 ;set code segment to 0x7c0

step2:
    ;Dont rely BIOS set 0x7c0 for us
    cli ;disable  interrups
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax  
    mov sp, 0x7c00
    sti ;enable interrups

.load_protected:
    cli
    lgdt[gdt_descripter]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

;GDT
gdt_start:
gdt_null:   ;64bits of 0
    dd 0x0
    dd 0x0

;offset 0x8 from gdt_start since gdt_null is 8bytes
gdt_code:   ;CS should point to this
    dw 0xffff   ;segment limit first 0~15 bits
    dw 0    ;base first 0~15bits
    db 0    ;base 16~23 bits
    db 0x9a ;access byte
    db 11001111b    ;high 4 bits flags and low 4 bits flag
    db 0    ;base 24~31bits

;offset 0x10
gdt_data:   ; link to DS SS ES FS GS
    dw 0xffff   ;segment limit first 0~15 bits
    dw 0    ;base first 0~15bits
    db 0    ;base 16~23 bits
    db 0x92 ;access byte
    db 11001111b    ;high 4 bits flags and low 4 bits flag
    db 0    ;base 24~31bits
gdt_end:

gdt_descripter:
    dw gdt_end - gdt_start-1
    dd gdt_start

[BITS 32]
load32:     ;load kernel to addr 0x0100000
    mov eax, 1 ;sector 1
    mov ecx, 100 ;reapet 100 times
    mov edi, 0x0100000 ;destinate addr
    call ata_lba_read   ;start to ata read

    ;once we are done reading kernel to addr 0x0100000, we can jmp there
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax, ; Backup the LBA
    ; Send the highest 8 bits of the lba to hard disk controller
    shr eax, 24
    or eax, 0xE0 ; Select the  master drive
    mov dx, 0x1F6
    out dx, al
    ; Finished sending the highest 8 bits of the lba

    ; Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending the total sectors to read

    ; Send more bits of the LBA
    mov eax, ebx ; Restore the backup LBA
    mov dx, 0x1F3
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx ; Restore the backup LBA
    shr eax, 8
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx ; Restore the backup LBA
    shr eax, 16
    out dx, al
    ; Finished sending upper 16 bits of the LBA

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx

; Checking if we need to read
.try_again:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

; We need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ; End of reading sectors into memory
    ret


times 510 - ($ - $$) db 0   ; $ means current addr, $$ means segment addr
;dw 0xAA55   ;remember intel use little endian
db 0x55
db 0xAA
