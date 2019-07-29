The Moira service management system.  See moira/README for build instructions.
On Ubuntu/Debian
sudo apt install comerr-dev libkrb5-dev libreadline-dev libhesiod-dev
Then
cd moira/moira && ./configure --with-krb5 --with-com_err --with-afs --with-hesiod --with-readline --without-zephyr --without-java --prefix=/usr
