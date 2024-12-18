gcc -std=c11 src/main.c src/cybertyper_core.c src/hal_mock.c -o cybertyper_test
stty -ixon
./cybertyper_test