# SimpleChessEngine
![](https://github.com/Matveiiy/SimpleChessEngine/blob/main/logo.png)

SCE is a chess engine written in educational purposes. Its rating is aproximatly 1690 - 1750 elo. SCE has won shallow blue and tscp It uses UCI protocol to communicate with GUI interface. It was not tested on linux!
### Installation
#### Windows
download and install latest release
#### Linux
All code is in one file. Just use makefile or any other tool you want to compile it
### License
Project uses [WTFPL](http://www.wtfpl.net/) license
### Features
- UCI protocol
- Bitboard board representation
- Encoding moves as integers
- Negamax algorithm with alpha beta prunning
- PV/killer/history/hash move ordering
- Iterative deepening
- Null move and static evaluation prunning 
- Razoring
- PVS (principle variation search)
- LMR (late move reduction)
- Simple tapered evaluation

