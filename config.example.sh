# Copy to config.local.sh and fill in. config.local.sh is not committed.

# Your PS5's IP address, for deploying over FTP (ftpsrv, port 2121) and sending payloads (elfldr, port 9021).
PS5_HOST=192.168.1.100

# Folder of your Might and Magic VI installation: the one that contains DATA, Anims and SOUNDS.
MM6_DATA="$HOME/Games/Might and Magic 6"

# Where LLVM 21 is installed, if not /usr/lib/llvm21 (Arch's llvm21/clang21/lld21 packages). On Debian/Ubuntu
# with apt.llvm.org packages this is /usr/lib/llvm-21.
#LLVM21=/usr/lib/llvm-21
