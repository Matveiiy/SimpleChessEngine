# SimpleChessEngine
![](https://github.com/Matveiiy/SimpleChessEngine/blob/main/logo.png)
###### Thanks to Vadim for logo

SCE is a chess engine written in educational purposes. Its rating is aproximatly 1690 - 1750 elo. SCE has won shallow blue and tscp It uses UCI protocol to communicate with GUI interface. It was not tested on linux!
### Installation
#### Windows
download and install latest release
#### Linux
All code is in one file. Just use makefile or any other tool you want to compile it
### License
Project uses [WTFPL](http://www.wtfpl.net/) license
### Features
- [x] UCI protocol
- [x] Bitboard board representation
- [x] Encoding moves as integers
- [x] Negamax algorithm with alpha beta prunning
- [x] PV/killer/history/hash move ordering
- [x] Iterative deepening
- [x] Null move prunning
- [x] PVS (principle variation search)
- [x] LMR (late move reduction)
- [x] Razoring 
- [x] Futility prunning, reverse futility prunning
- [ ] Tapered evaluation 

