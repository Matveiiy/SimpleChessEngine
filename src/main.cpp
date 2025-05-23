#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <atomic>
#include <thread>
#include <cmath>
#include <sstream>
#include <ext/pb_ds/assoc_container.hpp>
#ifdef WIN64
    #include <windows.h>
#else
    # include <sys/time.h>
#endif
typedef uint64_t bitboard;
typedef uint32_t Move;
typedef uint64_t hash_key; 
#define set_bit(bbitboard, square) ((bbitboard) |= (1ULL << (square)))
//#define get_bit(bbitboard, square) ((bbitboard) & (1ULL << (square)))
#define pop_bit(bbitboard, square) ((bbitboard) &= ~(1ULL << (square)))
inline bool get_bit(bitboard bb, int bit) {return (bb) & (1ULL << (bit));}
inline int bit_scan_forward(const auto& x) { return __builtin_ctzll(x);}
inline int count_bits(const auto& x) {return __builtin_popcountll(x); }
inline int get_time_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    /*
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
    */
}
inline void reset_lsb(auto& x) {x&=x-1;}
inline void print_bitboard(bitboard bb) {
    std::cout << "------------------\n";
    for (int i=7;i>=0;--i) {
        for (int j = 0; j < 8; ++j) {
            if (get_bit(bb, i*8+j)) std::cout << "|1";
            else std::cout << "|0"; 
        }
        std::cout << "|\n";
    }
    std::cout << "------------------\n";
}
template<typename ... Args>
inline std::string string_format(const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}
inline constexpr Move NullMove = 0;
inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
// pseudo random number state
inline unsigned int random_state = 1804289383;
inline unsigned int get_random_U32_number()
{
    unsigned int number = random_state;
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    random_state = number;
    return number;
}
inline hash_key get_random_U64_number()
{
    hash_key n1, n2, n3, n4;
    n1 = (hash_key)(get_random_U32_number()) & 0xFFFF;
    n2 = (hash_key)(get_random_U32_number()) & 0xFFFF;
    n3 = (hash_key)(get_random_U32_number()) & 0xFFFF;
    n4 = (hash_key)(get_random_U32_number()) & 0xFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

namespace ChessEngine
{
    static constexpr const char* startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    static constexpr bitboard AllSquares = -1;
    static constexpr bitboard DarkSquares = 0xAA55AA55AA55AA55UL;
    static constexpr bitboard FileABB = 0x0101010101010101UL;
    static constexpr bitboard FileBBB = FileABB << 1;
    static constexpr bitboard FileCBB = FileABB << 2;
    static constexpr bitboard FileDBB = FileABB << 3;
    static constexpr bitboard FileEBB = FileABB << 4;
    static constexpr bitboard FileFBB = FileABB << 5;
    static constexpr bitboard FileGBB = FileABB << 6;
    static constexpr bitboard FileHBB = FileABB << 7;


    static constexpr bitboard NotAFile = ~FileABB;
    static constexpr bitboard NotBFile = ~FileBBB;
    static constexpr bitboard NotCFile = ~FileCBB;
    static constexpr bitboard NotDFile = ~FileDBB;
    static constexpr bitboard NotEFile = ~FileEBB;
    static constexpr bitboard NotFFile = ~FileFBB;
    static constexpr bitboard NotGFile = ~FileGBB;
    static constexpr bitboard NotHFile = ~FileHBB;

    static constexpr bitboard Rank1BB = 0xFF;
    static constexpr bitboard Rank2BB = Rank1BB << (8 * 1);
    static constexpr bitboard Rank3BB = Rank1BB << (8 * 2);
    static constexpr bitboard Rank4BB = Rank1BB << (8 * 3);
    static constexpr bitboard Rank5BB = Rank1BB << (8 * 4);
    static constexpr bitboard Rank6BB = Rank1BB << (8 * 5);
    static constexpr bitboard Rank7BB = Rank1BB << (8 * 6);
    static constexpr bitboard Rank8BB = Rank1BB << (8 * 7);

    static constexpr bitboard Not8Rank = ~Rank8BB;
    static constexpr bitboard Not1Rank = ~Rank1BB;
    enum class Piece { E, P, N, B, R, Q, K };
    enum class ColorPiece { E, WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK };
    enum class CastleEnum { E=0, WQ, WK, BQ, BK };
    enum class Color {WHITE, BLACK};
    enum class MoveType
    {
        NORMAL = 0,
        PROMOTION = 1,
        EN_PASSANT = 2,
        CASTLES,
    };
    enum class Square:int
    {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
    };
    struct MoveList{
        Move move_storage[256];
        int count=0;
        void AddMove(Move move) {move_storage[count++] = move;}
    };
    /*
    32 bits
    2 bits - type 
    6 bits - from
    6 bits - to
    4 bits captured
    4 bits - promotion if needed/other
    4 bits - castling rights
    */
    inline Move EncodeMove(MoveType type, Square from, Square to, uint8_t capture = 0, uint8_t param = 0, Square en_passant_square = Square::A1, int castle = 0) { 
        return ((Move)type << 30) 
        | ((Move)from << 24) 
        | ((Move)to << 18) 
        | ((Move)capture << 14) 
        | (param << 10) 
        | ((int)en_passant_square << 4)
        | castle; }
    inline MoveType GetMoveType(Move move) { return (MoveType)(move >> 30);}
    inline Square GetMoveFrom(Move move)            {return (Square)((move >> 24) & 0b00111111); } // seems to work}
    inline Square GetMoveTo(Move move)              {return (Square)((move >> 18) & 0b00000000111111); } // seems to work}
    inline uint8_t GetMoveCapture(Move move)       {return (uint8_t)((move >> 14) & 0b000000000000001111); } // seems to work}
    inline uint8_t GetMoveParam(Move move)        {return (uint8_t)((move >>  10) & 0b0000000000000000001111);} // seems to work}
    inline Square GetMoveEnPassantFlag(Move move)   {return (Square)((move >>  4) & 0b0000000000000000000000111111);} // seems to work}
    inline int GetMoveCastleFlag(Move move)                       {return ((move) & 0b00000000000000000000000000001111);}
    inline bool MoveEquals(Move move, Square from, Square to) {return (GetMoveFrom(move) == from) && (GetMoveTo(move) == to);}
    /*
    inline bool GetMoveChangeWQ(Move move) {return (bool)((move >>  3) & 0b00000000000000000000000000001);} // seems to work}
    inline bool GetMoveChangeWK(Move move) {return (bool)((move >>  2) & 0b000000000000000000000000000001);} // seems to work}
    inline bool GetMoveChangeBQ(Move move) {return (bool)((move >>  1) & 0b0000000000000000000000000000001);} // seems to work}
    inline bool GetMoveChangeBK(Move move) {return (bool)((move      ) & 0b00000000000000000000000000000001);} // seems to work}
    */
    inline const char* square_to_string(Square pos) {constexpr const char* conv[] = {"A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1","A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2","A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3","A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4","A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5","A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6","A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7","A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",};return conv[(int)pos];}
    inline const char* square_to_string_tolower(Square pos) {constexpr const char* conv[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1","a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2","a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3","a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4","a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5","a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6","a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7","a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",};return conv[(int)pos];}
    inline const char* movetype_to_string(MoveType type) {switch(type) {case MoveType::NORMAL: return "Normal";case MoveType::CASTLES: return "Castle";case MoveType::EN_PASSANT: return "En passant";case MoveType::PROMOTION: return "Promotion";}}
    inline constexpr char piece_chars[13] = {' ', 'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    inline std::string move_to_string(const Move& move) {
        if (GetMoveType(move) == MoveType::EN_PASSANT) {
            std::string str = square_to_string_tolower(GetMoveFrom(move));
            if ((int)GetMoveTo(move)/8 == 4) str += square_to_string_tolower((Square)((int)GetMoveTo(move) + 8));
            else str += square_to_string_tolower((Square)((int)GetMoveTo(move) - 8));
            return str;
        }
        std::string str = square_to_string_tolower(GetMoveFrom(move));str += square_to_string_tolower(GetMoveTo(move));
        if (GetMoveParam(move)) {
            if (GetMoveParam(move) > 6) 
                str += piece_chars[GetMoveParam(move)];
            else 
                str += piece_chars[GetMoveParam(move) + 6];
        }
        return str;
    }
    inline void print_move(Move move) {std::cout << move_to_string(move);}
    inline void print_move_std(Move move) {std::cout << move_to_string(move);}
    inline uint32_t hash_size = 1000000;
    static constexpr uint32_t max_hash_size = 100000000;
    static constexpr int NO_HASH = 100000;
    static constexpr int HASHF_EXACT = 0;
    static constexpr int HASHF_ALPHA = 1;
    static constexpr int HASHF_BETA = 2;
    struct tagHASH{
        Move best = NullMove;  
        int depth = -1;      
        int flags;       
        int score;  
    };
    uint64_t hash_hit = 0;
    inline hash_key repetition_table[700] = {};
    inline int repetition_index = 0;
    inline __gnu_pbds::gp_hash_table<hash_key, tagHASH> hash_table;
    //inline tagHASH* hash_table;
    inline bool is_repetition(hash_key key) {for (int i = 0; i < repetition_index; i++) {if (key == repetition_table[i]) return true;}return false;}
    inline void clear_hash_table() {hash_table.clear();}
    inline int read_hash_entry(int depth, auto& key, int alpha, int beta, Move& best) {
        tagHASH& phase = hash_table[key];
        hash_hit++;
        if (phase.depth >= depth) {
            if (phase.flags == HASHF_EXACT)
                return phase.score;
            if ((phase.flags == HASHF_ALPHA) &&
                (phase.score <= alpha))
                return alpha;
            if ((phase.flags == HASHF_BETA) &&
                (phase.score >= beta))
                return beta;
        }
        hash_hit--;
        best = phase.best;
        return NO_HASH;
    }
    inline void write_hash_entry(int depth, auto& key, int val, int hashf, Move best) {
        tagHASH& phashe = hash_table[key];
        phashe.score = val;
        phashe.flags = hashf;
        phashe.depth = depth;
        phashe.best = best;
    }
    static inline constexpr int MAX_PLY = 64;
    struct Board{
    public:
        Board() {}
        ~Board() {}
        inline bool IsWhiteToMove() {return move_count&1;}
        template<bool IsWhite>
        inline hash_key GenHashKey() {
            hash_key newkey = 0;
            for (int i = 0; i < 64; ++i) {
                if ((int)piece_board[i]) newkey ^=piece_keys[(int)piece_board[i]-1][i];
            }
            //castling & en passant
            newkey^=castle_keys[castle];
            newkey^=enpassant_keys[(int)en_passant];
            //hash turn
            if constexpr (IsWhite) newkey^=side_key;
            return newkey;
        }
        inline void Reset() {
            memset(bboard, 0ULL, sizeof(bboard));
            memset(piece_board, 0ULL, sizeof(piece_board));
            move_count = 1;
            castle = 0;
            current_key = 0;
            en_passant = Square::A1;
        }
        inline void InitBoard() {}
        void ShowBoard() {
            std::cout << "-----------------\n";
            for (int i = 7; i >= 0; --i)
            {
                for (int j = 0; j < 8; j++)
                {
                    std::cout << '|' << piece_chars[(int)piece_board[i*8+j]];
                }
                std::cout << "|\n";           
                std::cout << "-----------------\n";
            }
        }
        bool SetFen(std::string fen) {
            memset(bboard, 0ULL, sizeof(bboard));
            memset(repetition_table, 0ULL, sizeof(repetition_table));
            for (int i =0;i<64;++i) piece_board[i] = (ColorPiece)0;
            repetition_index = -1;
            castle = 0;
            en_passant = Square::A1;
            int pos = 0, i, j; 
            for (i=7;i>=0;--i) {
                for (j=0;j<8;++j) {
                    for (int k = 1; k < 13; ++k) {
                        if (fen[pos] == piece_chars[k]) {
                            piece_board[i*8+j] = (ColorPiece)(k);
                            set_bit(bboard[k], i*8+j);
                            pos++;
                            break; 
                        }
                    }
                    if ((int)piece_board[i*8+j]) continue;
                    int num = fen[pos] - '1';
                    if (num < 0 || num > 8) return false;
                    j+=num;
                    pos++;
                }
                if (j!=8) return false;
                if (i == 0) {
                    if (fen[pos] != ' ') return false;
                    ++pos;
                }
                else {
                    if (fen[pos] == '/') ++pos;
                    else return false;
                }
                //std::cout << 3;
            }
            if (fen[pos] == 'w') move_count = 1;
            else if (fen[pos] == 'b') move_count = 2;
            else return false;
            ++pos;
            if (fen[pos++] != ' ') return false;
            if (fen[pos] == '-') {
                ++pos;
                if (fen[pos++] != ' ') return false;
            }
            else {
                while (fen[pos]!=' ') {
                    if (fen[pos] == 'Q') castle |= 8;
                    else if (fen[pos] == 'K') castle |= 4;
                    else if (fen[pos] == 'k') castle |= 1;
                    else if (fen[pos] == 'q') castle |= 2;
                    else return false;
                    pos++;
                } 
                ++pos;
            }
            if (fen[pos] == '-') {
                if (fen[++pos] != ' ') return false;
            }
            else {
                if (fen[pos] < 'a' || fen[pos] > 'h') return false;
                int x = fen[pos++] - 'a';
                if (fen[pos] == '3') {
                    en_passant = (Square)(24+x);
                }
                else if (fen[pos] == '6') {
                    en_passant = (Square)(32+x);
                }
                else return false;
                if (fen[++pos] != ' ') return false;
            }
            if (move_count & 1) current_key = GenHashKey<true>();
            else current_key = GenHashKey<false>();
            return true;
        }
        std::string GetFen() {
            int space = 0;
            std::string fen = "";
            for (int y = 7;y >= 0; --y)
            {
                for (int x = 0; x < 8; ++x)
                {
                    if (piece_board[y * 8 + x] == ColorPiece::E) space++;
                        else
                        {
                            if (space == 0) fen+=piece_chars[(int)piece_board[y*8+x]];
                            else {fen+=char(space+'0');fen+=piece_chars[(int)piece_board[y*8+x]];}
                            space = 0;
                        }
                }
                if (space != 0) { fen += char(space+'0');space = 0; }
                fen += "/";
            }
            fen[fen.size()-1] = ' ';
            if (move_count&1) fen+='w';
            else fen+='b';
            fen+=' ';
            if (castle == 0) fen+='-';
            else {
                if (castle&4) fen+='K';
                if (castle&8) fen+='Q';
                if (castle&1) fen+='k';
                if (castle&2) fen+='q';
            }
            fen += " - ";
            return fen;
        }
        inline void UpdateOccupancy() {cur_all = bboard[1] | bboard[2] | bboard[3] | bboard[4] | bboard[5] | bboard[6] | bboard[12] | bboard[11] | bboard[10] | bboard[9] | bboard[8] | bboard[7];}
        template<bool IsWhite> inline int HowManyLegalMoves() {
            MoveList moves;
            GenMovesUnchecked<IsWhite>(moves);
            int count = 0;
            for (int i = 0; i < moves.count; ++i) {
                MakeMove<IsWhite>(moves.move_storage[i]);
                if (!IsKingInCheck<IsWhite>()) ++count;
                UndoMove<IsWhite>(moves.move_storage[i]);
            }
            return count;
        }
        template<bool IsWhite> void UpdateOccupanciesColors() {
            cur_all = bboard[1] | bboard[2] | bboard[3] | bboard[4] | bboard[5] | bboard[6] | bboard[12] | bboard[11] | bboard[10] | bboard[9] | bboard[8] | bboard[7];
            if constexpr (IsWhite) {
                not_own = ~(bboard[1] | bboard[2] | bboard[3] | bboard[4] | bboard[5] | bboard[6]);
                enemy = ( bboard[12] | bboard[11] | bboard[10] | bboard[9] | bboard[8] | bboard[7]);
            }
            else {
                not_own = ~(bboard[12] | bboard[11] | bboard[10] | bboard[9] | bboard[8] | bboard[7]);
                enemy = ( bboard[1] | bboard[2] | bboard[3] | bboard[4] | bboard[5] | bboard[6]);
            }
        }
        template<bool IsWhite> void GenMovesUnchecked(MoveList& movelist) {
            if constexpr (IsWhite) {
                not_own = ~(bboard[1] | bboard[2] | bboard[3] | bboard[4] | bboard[5] | bboard[6]);
                enemy = ( bboard[12] | bboard[11] | bboard[10] | bboard[9] | bboard[8] | bboard[7]);
                cur_all = (~not_own) | enemy;
                GenWhitePawnMoves(movelist);
            }
            else {
                not_own = ~(bboard[12] | bboard[11] | bboard[10] | bboard[9] | bboard[8] | bboard[7]);
                enemy = (bboard[1] | bboard[2] | bboard[3] | bboard[4] | bboard[5] | bboard[6]);
                cur_all = (~not_own) | enemy;
                GenBlackPawnMoves(movelist);
            }
            GenRookMoves<IsWhite>(movelist);
            //std::cout << movelist.count;
            GenBishopMoves<IsWhite>(movelist);
            GenQueenMoves<IsWhite>(movelist);
            GenKnightMoves<IsWhite>(movelist);
            GenKingMoves<IsWhite>(movelist);
            GenCastleQueenside<IsWhite>(movelist);
            UpdateOccupancy();
            GenCastleKingside<IsWhite>(movelist);
        }
        template<bool WhiteKing> bool IsKingInCheck(bitboard enemy_mask) {if (WhiteKing) return (enemy_mask & bboard[(int)ColorPiece::WK]);else return (enemy_mask & bboard[(int)ColorPiece::BK]);} 
        template<bool IsWhite> bitboard GetAttackMask() {
            bitboard attack_mask, temp;
            if constexpr (IsWhite)  attack_mask = ((bboard[(int)ColorPiece::WP] << 9) & NotAFile) | ((bboard[(int)ColorPiece::WP] << 7) & NotHFile);
            else attack_mask = ((bboard[(int)ColorPiece::BP] >> 9) & NotHFile) | ((bboard[(int)ColorPiece::BP] >> 7) & NotAFile); 
            
            if constexpr (IsWhite) temp = bboard[(int)ColorPiece::WB];
            else temp = bboard[(int)ColorPiece::BB];
            while (temp)
            {
                attack_mask |= BishopAttacks(bit_scan_forward(temp), cur_all);
                reset_lsb( temp);
            }
            if constexpr (IsWhite) temp = bboard[(int)ColorPiece::WR];
            else temp = bboard[(int)ColorPiece::BR];
            while (temp)
            {
                attack_mask |= RookAttacks(bit_scan_forward(temp), cur_all);
                reset_lsb( temp);
            }
            if constexpr (IsWhite) temp = bboard[(int)ColorPiece::WQ];
            else temp = bboard[(int)ColorPiece::BQ];
            while (temp)
            {
                attack_mask |= QueenAttacks(bit_scan_forward(temp), cur_all);
                reset_lsb( temp);
            }
            if constexpr (IsWhite) temp = bboard[(int)ColorPiece::WN];
            else temp = bboard[(int)ColorPiece::BN];
            while (temp)
            {
                attack_mask |= KnightAttacks(bit_scan_forward(temp));
                reset_lsb( temp);
            }
            if constexpr (IsWhite) temp = bboard[(int)ColorPiece::WK];
            else temp = bboard[(int)ColorPiece::BK];
            while (temp)
            {
                attack_mask |= KingAttacks(bit_scan_forward(temp));
                reset_lsb( temp);
            }
            //while ()
            return attack_mask;
        }
        template<bool WhiteKing> bool IsKingInCheck() {if constexpr(WhiteKing) return IsKingInCheck<WhiteKing>(GetAttackMask<false>());else return IsKingInCheck<WhiteKing>(GetAttackMask<true>());        }
        template<bool WhiteMoves> inline void MakeMove(Move& move) {
            const int from = (int)GetMoveFrom(move);
            const int to = (int)GetMoveTo(move);
            switch (GetMoveType(move))
            {
                //takes (castle: 4) + 6+6+4 = 20 byte at least
                case MoveType::NORMAL:{
                    const auto captured = GetMoveCapture(move);
                    current_key^=piece_keys[(int)piece_board[from]-1][to];
                    current_key^=piece_keys[(int)piece_board[from]-1][from];
                    if (captured) current_key^=piece_keys[captured-1][to];
                    set_bit(bboard[(int)piece_board[from]], to);
                    pop_bit(bboard[(int)piece_board[from]], from);
                    pop_bit(bboard[captured], to);
                    piece_board[to] = piece_board[from];
                    piece_board[from] = ColorPiece::E;
                    break;
                }
                case MoveType::CASTLES:{
                    switch (GetMoveTo(move)) {
                        case Square::C1:
                            current_key^=piece_keys[(int)ColorPiece::WK-1][(int)Square::E1];
                            current_key^=piece_keys[(int)ColorPiece::WR-1][(int)Square::A1];
                            current_key^=piece_keys[(int)ColorPiece::WK-1][(int)Square::C1];
                            current_key^=piece_keys[(int)ColorPiece::WR-1][(int)Square::D1];
                            pop_bit(bboard[(int)ColorPiece::WK], (int)Square::E1);
                            pop_bit(bboard[(int)ColorPiece::WR], (int)Square::A1);
                            set_bit(bboard[(int)ColorPiece::WK], (int)Square::C1);
                            set_bit(bboard[(int)ColorPiece::WR], (int)Square::D1);
                            piece_board[(int)Square::E1] = ColorPiece::E;
                            piece_board[(int)Square::D1] = ColorPiece::WR;
                            piece_board[(int)Square::C1] = ColorPiece::WK;
                            piece_board[(int)Square::A1] = ColorPiece::E;
                            break;
                        case Square::G1:
                            current_key^=piece_keys[(int)ColorPiece::WK-1][(int)Square::E1];
                            current_key^=piece_keys[(int)ColorPiece::WR-1][(int)Square::H1];
                            current_key^=piece_keys[(int)ColorPiece::WK-1][(int)Square::G1];
                            current_key^=piece_keys[(int)ColorPiece::WR-1][(int)Square::F1];
                            pop_bit(bboard[(int)ColorPiece::WK], (int)Square::E1);
                            pop_bit(bboard[(int)ColorPiece::WR], (int)Square::H1);
                            set_bit(bboard[(int)ColorPiece::WK], (int)Square::G1);
                            set_bit(bboard[(int)ColorPiece::WR], (int)Square::F1);
                            piece_board[(int)Square::E1] = ColorPiece::E;
                            piece_board[(int)Square::F1] = ColorPiece::WR;
                            piece_board[(int)Square::G1] = ColorPiece::WK;
                            piece_board[(int)Square::H1] = ColorPiece::E;
                            break;
                        case Square::C8:
                            current_key^=piece_keys[(int)ColorPiece::BK-1][(int)Square::E8];
                            current_key^=piece_keys[(int)ColorPiece::BR-1][(int)Square::A8];
                            current_key^=piece_keys[(int)ColorPiece::BK-1][(int)Square::C8];
                            current_key^=piece_keys[(int)ColorPiece::BR-1][(int)Square::D8];
                            pop_bit(bboard[(int)ColorPiece::BK], (int)Square::E8);
                            pop_bit(bboard[(int)ColorPiece::BR], (int)Square::A8);
                            set_bit(bboard[(int)ColorPiece::BK], (int)Square::C8);
                            set_bit(bboard[(int)ColorPiece::BR], (int)Square::D8);
                            piece_board[(int)Square::E8] = ColorPiece::E;
                            piece_board[(int)Square::D8] = ColorPiece::BR;
                            piece_board[(int)Square::C8] = ColorPiece::BK;
                            piece_board[(int)Square::A8] = ColorPiece::E;
                            break;
                        case Square::G8:
                            current_key^=piece_keys[(int)ColorPiece::BK-1][(int)Square::E8];
                            current_key^=piece_keys[(int)ColorPiece::BR-1][(int)Square::H8];
                            current_key^=piece_keys[(int)ColorPiece::BK-1][(int)Square::G8];
                            current_key^=piece_keys[(int)ColorPiece::BR-1][(int)Square::F8];
                            pop_bit(bboard[(int)ColorPiece::BK], (int)Square::E8);
                            pop_bit(bboard[(int)ColorPiece::BR], (int)Square::H8);
                            set_bit(bboard[(int)ColorPiece::BK], (int)Square::G8);
                            set_bit(bboard[(int)ColorPiece::BR], (int)Square::F8);
                            piece_board[(int)Square::E8] = ColorPiece::E;
                            piece_board[(int)Square::F8] = ColorPiece::BR;
                            piece_board[(int)Square::G8] = ColorPiece::BK;
                            piece_board[(int)Square::H8] = ColorPiece::E;
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case MoveType::EN_PASSANT:{
                    if constexpr (WhiteMoves) {
                        const auto captured = to + 8;
                        const auto param = GetMoveCapture(move);
                        current_key^=piece_keys[(int)piece_board[from]-1][captured];
                        current_key^=piece_keys[(int)piece_board[from]-1][from];
                        current_key^=piece_keys[param-1][to];
                        set_bit(bboard[(int)piece_board[from]], captured);
                        pop_bit( bboard[(int)piece_board[from]], from);
                        pop_bit(bboard[param], to);
                        piece_board[captured] = piece_board[from];
                        piece_board[from] = ColorPiece::E;
                        piece_board[to] = ColorPiece::E;
                    }
                    else {
                        const auto captured = to - 8;
                        const auto param = GetMoveCapture(move);
                        current_key^=piece_keys[(int)piece_board[from]-1][captured];
                        current_key^=piece_keys[(int)piece_board[from]-1][from];
                        current_key^=piece_keys[param-1][to];
                        set_bit(bboard[(int)piece_board[from]], captured);
                        pop_bit( bboard[(int)piece_board[from]], from);
                        pop_bit(bboard[param], to);
                        piece_board[captured] = piece_board[from];
                        piece_board[from] = ColorPiece::E;
                        piece_board[to] = ColorPiece::E;
                    }
                    //current_key^=piece_keys[(int)piece_board[from]][to];
                    //current_key^=piece_keys[(int)piece_board[from]][from];
                    //current_key^=piece_keys[param][captured];
                    break;
                }
                case MoveType::PROMOTION:{
                    const auto captured = GetMoveCapture(move);
                    const auto promoted = GetMoveParam(move);
                    current_key^=piece_keys[promoted-1][to];
                    current_key^=piece_keys[(int)piece_board[from]-1][from];
                    if (captured) current_key^=piece_keys[captured-1][to];
                    set_bit(bboard[promoted], to);
                    pop_bit(bboard[(int)piece_board[from]], from);
                    pop_bit(bboard[captured], to);
                    piece_board[to] = (ColorPiece)promoted;
                    piece_board[from] = ColorPiece::E;
                    break;
                }
            }
            //-----save state------
            //current_key^=castle_keys[castle];
            //en_passant = GetMoveEnPassantFlag(move);
            current_key^=enpassant_keys[(int)en_passant];//remove current en passant
            current_key^=castle_keys[castle];//remove castling

            auto temp = en_passant;
            en_passant = GetMoveEnPassantFlag(move);
            move = EncodeMove(GetMoveType(move), (Square)from, (Square)to, GetMoveCapture(move), GetMoveParam(move), temp, castle);
            //-----save state------
            
            //update stateGenBlack
            castle &= castling_rights[from];
            castle &= castling_rights[to];
            //restore castling & en passant
            current_key^=enpassant_keys[(int)en_passant];
            current_key^=castle_keys[castle];
            current_key^=side_key;
            
            UpdateOccupancy();
            ++move_count;
            //current_key = GenHashKey<WhiteMoves>();
            //Board.print_move(move);
        }
        template<bool WhiteMoves> inline void UndoMove(Move& move) {
            --move_count;
            current_key^=side_key;
            const int from = (int)GetMoveFrom(move);
            const int to = (int)GetMoveTo(move);
            current_key^=enpassant_keys[(int)en_passant];
            current_key^=castle_keys[castle];
            castle = GetMoveCastleFlag(move);
            en_passant = GetMoveEnPassantFlag(move);
            current_key^=enpassant_keys[(int)en_passant];
            current_key^=castle_keys[castle];
            switch (GetMoveType(move))
            {
                case MoveType::NORMAL:{
                        const int captured = GetMoveCapture(move);
                        current_key^=piece_keys[(int)piece_board[to]-1][from];
                        current_key^=piece_keys[(int)piece_board[to]-1][to];
                        if (captured) current_key^=piece_keys[captured-1][to];
                        set_bit(bboard[(int)piece_board[to]], from);
                        pop_bit(bboard[(int)piece_board[to]], to);
                        set_bit(bboard[captured], to);
                        piece_board[from] = piece_board[to];
                        piece_board[to] = (ColorPiece)captured;
                    }
                    //can_castle[GetMoveParam2(move)] = true;
                    break;
                case MoveType::EN_PASSANT:{
                    if constexpr (WhiteMoves) {
                        const int captured = to+8;
                        auto piece = GetMoveCapture(move);
                        current_key^=piece_keys[(int)piece_board[captured]-1][from];
                        current_key^=piece_keys[(int)piece_board[captured]-1][captured];
                        current_key^=piece_keys[piece-1][to];
                        set_bit(bboard[(int)piece_board[captured]], from);
                        pop_bit(bboard[(int)piece_board[captured]], captured);
                        set_bit(bboard[piece], to);
                        piece_board[from] = piece_board[captured];
                        piece_board[captured] = ColorPiece::E;
                        piece_board[to] = (ColorPiece)piece;
                    }
                    else {
                        const int captured = to-8;
                        auto piece = GetMoveCapture(move);
                        current_key^=piece_keys[(int)piece_board[captured]-1][from];
                        current_key^=piece_keys[(int)piece_board[captured]-1][captured];
                        current_key^=piece_keys[piece-1][to];
                        set_bit(bboard[(int)piece_board[captured]], from);
                        pop_bit(bboard[(int)piece_board[captured]], captured);
                        set_bit(bboard[piece], to);
                        piece_board[from] = piece_board[captured];
                        piece_board[captured] = ColorPiece::E;
                        piece_board[to] = (ColorPiece)piece;
                    }
                    break;
                }
                case MoveType::CASTLES: 
                    switch (GetMoveTo(move)) {
                        case Square::C1:
                            current_key^=piece_keys[(int)ColorPiece::WK - 1][(int)Square::E1];
                            current_key^=piece_keys[(int)ColorPiece::WR - 1][(int)Square::A1];
                            current_key^=piece_keys[(int)ColorPiece::WK - 1][(int)Square::C1];
                            current_key^=piece_keys[(int)ColorPiece::WR - 1][(int)Square::D1];
                            set_bit(bboard[(int)ColorPiece::WK], (int)Square::E1);
                            set_bit(bboard[(int)ColorPiece::WR], (int)Square::A1);
                            pop_bit(bboard[(int)ColorPiece::WK], (int)Square::C1);
                            pop_bit(bboard[(int)ColorPiece::WR], (int)Square::D1);
                            piece_board[(int)Square::E1] = ColorPiece::WK;
                            piece_board[(int)Square::D1] = ColorPiece::E;
                            piece_board[(int)Square::C1] = ColorPiece::E;
                            piece_board[(int)Square::A1] = ColorPiece::WR;
                            break;
                        case Square::G1:
                            current_key^=piece_keys[(int)ColorPiece::WK - 1][(int)Square::E1];
                            current_key^=piece_keys[(int)ColorPiece::WR - 1][(int)Square::H1];
                            current_key^=piece_keys[(int)ColorPiece::WK - 1][(int)Square::G1];
                            current_key^=piece_keys[(int)ColorPiece::WR - 1][(int)Square::F1];
                            set_bit(bboard[(int)ColorPiece::WK], (int)Square::E1);
                            set_bit(bboard[(int)ColorPiece::WR], (int)Square::H1);
                            pop_bit(bboard[(int)ColorPiece::WK], (int)Square::G1);
                            pop_bit(bboard[(int)ColorPiece::WR], (int)Square::F1);
                            piece_board[(int)Square::E1] = ColorPiece::WK;
                            piece_board[(int)Square::F1] = ColorPiece::E;
                            piece_board[(int)Square::G1] = ColorPiece::E;
                            piece_board[(int)Square::H1] = ColorPiece::WR;
                            break;
                        case Square::C8:
                            current_key^=piece_keys[(int)ColorPiece::BK - 1][(int)Square::E8];
                            current_key^=piece_keys[(int)ColorPiece::BR - 1][(int)Square::A8];
                            current_key^=piece_keys[(int)ColorPiece::BK - 1][(int)Square::C8];
                            current_key^=piece_keys[(int)ColorPiece::BR - 1][(int)Square::D8];
                            set_bit(bboard[(int)ColorPiece::BK], (int)Square::E8);
                            set_bit(bboard[(int)ColorPiece::BR], (int)Square::A8);
                            pop_bit(bboard[(int)ColorPiece::BK], (int)Square::C8);
                            pop_bit(bboard[(int)ColorPiece::BR], (int)Square::D8);
                            piece_board[(int)Square::E8] = ColorPiece::BK;
                            piece_board[(int)Square::D8] = ColorPiece::E;
                            piece_board[(int)Square::C8] = ColorPiece::E;
                            piece_board[(int)Square::A8] = ColorPiece::BR;
                            break;
                        case Square::G8:
                            current_key^=piece_keys[(int)ColorPiece::BK - 1][(int)Square::E8];
                            current_key^=piece_keys[(int)ColorPiece::BR - 1][(int)Square::H8];
                            current_key^=piece_keys[(int)ColorPiece::BK - 1][(int)Square::G8];
                            current_key^=piece_keys[(int)ColorPiece::BR - 1][(int)Square::F8];
                            set_bit(bboard[(int)ColorPiece::BK], (int)Square::E8);
                            set_bit(bboard[(int)ColorPiece::BR], (int)Square::H8);
                            pop_bit(bboard[(int)ColorPiece::BK], (int)Square::G8);
                            pop_bit(bboard[(int)ColorPiece::BR], (int)Square::F8);
                            piece_board[(int)Square::E8] = ColorPiece::BK;
                            piece_board[(int)Square::F8] = ColorPiece::E;
                            piece_board[(int)Square::G8] = ColorPiece::E;
                            piece_board[(int)Square::H8] = ColorPiece::BR;
                            break;
                        default:
                            break;
                    }
                    break;
                case MoveType::PROMOTION: {
                    const int captured = GetMoveCapture(move);
                    auto offset = (to > from) ? 0 : 6;
                    int promoted = GetMoveParam(move);
                    current_key^=piece_keys[promoted-1][to];
                    current_key^=piece_keys[offset][from];
                    if (captured) current_key^=piece_keys[captured-1][to];

                    pop_bit(bboard[promoted], to);
                    set_bit(bboard[(int)ColorPiece::WP+offset], from);
                    set_bit(bboard[captured], to);
                    piece_board[to] = (ColorPiece)captured;
                    piece_board[from] = (ColorPiece)((int)ColorPiece::WP + offset);
                    break;
                }
            }
            UpdateOccupancy();
            //current_key = GenHashKey<WhiteMoves>();
        }
    private:
        inline void GenWhitePawnMoves(MoveList& gmoves)
        {
            bitboard available1 = bboard[(int)ColorPiece::WP];
            bitboard available2 = available1;
            bitboard quiet = (available2 << 8) & ~cur_all;
            bitboard doublemove = ((available1 & Rank2BB) << 8) & ~cur_all;
            doublemove = (doublemove << 8) & ~cur_all;

            available1 = (available1 << 9) & NotAFile;
            available2 = (available2 << 7) & NotHFile;
            available1 &= enemy;
            available2 &= enemy;
            auto promoting1 = available1 & Rank8BB;
            auto promoting2 = available2 & Rank8BB;
            auto quiet_promote = quiet & Rank8BB;
            available1 &= Not8Rank;
            available2 &= Not8Rank;
            quiet &= Not8Rank;

            //Board.print_move(moves[move_count - 1]);
            //available = (available << 9);
            // 0 0 0   0 0 0   0 0 0
            // 0 1 1 & 0 1 0 = 0 1 0
            // 0 0 0   0 0 0   0 0 0 
            while (promoting2)
            {
                auto x = bit_scan_forward(promoting2);
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::Q));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::N));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::R));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::B));//todo: check
                reset_lsb(promoting2);
            }
            while (promoting1)
            {
                auto x = bit_scan_forward(promoting1);
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::Q));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::N));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::R));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)Piece::B));//todo: check
                reset_lsb(promoting1);
            }
            while (quiet_promote)
            {
                auto x = bit_scan_forward(quiet_promote);
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 8), (Square)x, 0, (uint8_t)Piece::Q));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 8), (Square)x, 0, (uint8_t)Piece::N));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 8), (Square)x, 0, (uint8_t)Piece::R));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x - 8), (Square)x, 0, (uint8_t)Piece::B));//todo: check
                reset_lsb(quiet_promote);
            }
            while (available2)
            {
                auto x = bit_scan_forward(available2);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x - 7), (Square)x, (uint8_t)piece_board[x]));//todo: check
                reset_lsb(available2);
            }
            while (available1)
            {
                auto x = bit_scan_forward(available1);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x - 9), (Square)x, (uint8_t)piece_board[x]));//todo: check
                reset_lsb(available1);
            }
            while (doublemove)
            {
                auto x = bit_scan_forward(doublemove);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x - 16), (Square)x, 0, 0, (Square)x));//todo: check
                reset_lsb(doublemove);
            }
            while (quiet)
            {
                auto x = bit_scan_forward(quiet);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x - 8), (Square)x));//todo: check
                reset_lsb(quiet);
            }
            //en_passant
            //const auto en_passant_move = move_history[move_count - 1];
            //Board.print_move(en_passant_move);
            //const auto to = (int)GetMoveTo(en_passant_move);
            //last is double pawn push
            //en_passant square equals last double pawn push to
            if ((int)en_passant/8 == 4) {
                const auto en_try1 = (int)en_passant - 1;
                const auto en_try2 = (int)en_passant + 1;
                //std::cout << en_try1 << ' ' << en_try2 << '\n';
                if (en_try2 / 8 == 4) if (get_bit(bboard[(int)Piece::P], (int)en_try2)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try2, (Square)((int)en_passant), (uint8_t)piece_board[(int)en_passant])); }
                if (en_try1 / 8 == 4) if (get_bit(bboard[(int)Piece::P], (int)en_try1)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try1, (Square)((int)en_passant), (uint8_t)piece_board[(int)en_passant])); }
            }
            /*
            if (piece_board[to] == ColorPiece::BP && ((int)GetMoveFrom(en_passant_move) / 8 == 6) && (to / 8 == 4)) {
                const auto en_try1 = to - 1;
                const auto en_try2 = to + 1;
                std::cout << en_try1 << ' ' << en_try2 << '\n';
                if (en_try2 / 8 == 4) if (get_bit(bboard[(int)Piece::P], (int)en_try2)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try2, (Square)to, (uint8_t)piece_board[to])); }
                if (en_try1 / 8 == 4) if (get_bit(bboard[(int)Piece::P], (int)en_try1)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try1, (Square)to, (uint8_t)piece_board[to])); }
            }
            */
            
            //available = (available << 9);
            // 0 0 0   0 0 0   0 0 0
            // 0 1 1 & 0 1 0 = 0 1 0
            // 0 0 0   0 0 0   0 0 0 
        }
        inline void GenBlackPawnMoves(MoveList& gmoves)
        {
            bitboard available1 = bboard[(int)ColorPiece::BP];
            bitboard available2 = available1;
            bitboard quiet = (available2 >> 8) & ~cur_all;
            bitboard doublemove = ((available1 & Rank7BB) >> 8) & ~cur_all;
            doublemove = (doublemove >> 8) & ~cur_all;

            available1 = (available1 >> 9) & NotHFile;
            available2 = (available2 >> 7) & NotAFile;
            available1 &= enemy;
            available2 &= enemy;
            auto promoting1 = available1 & Rank1BB;
            auto promoting2 = available2 & Rank1BB;
            auto quiet_promote = quiet & Rank1BB;
            available1 &= Not1Rank;
            available2 &= Not1Rank;
            quiet &= Not1Rank;

            //Board.print_move(moves[move_count - 1]);
            //available = (available << 9);
            // 0 0 0   0 0 0   0 0 0
            // 0 1 1 & 0 1 0 = 0 1 0
            // 0 0 0   0 0 0   0 0 0 

            while (promoting2)
            {
                auto x = bit_scan_forward(promoting2);
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BQ));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BN));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BR));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 7), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BB));//todo: check
                reset_lsb(promoting2);
            }
            while (promoting1)
            {
                auto x = bit_scan_forward(promoting1);
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BQ));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BN));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BR));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 9), (Square)x, (uint8_t)piece_board[x], (uint8_t)ColorPiece::BB));//todo: check
                reset_lsb(promoting1);
            }
            while (quiet_promote)
            {
                auto x = bit_scan_forward(quiet_promote);
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 8), (Square)x, 0, (uint8_t)ColorPiece::BQ));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 8), (Square)x, 0, (uint8_t)ColorPiece::BN));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 8), (Square)x, 0, (uint8_t)ColorPiece::BR));//todo: check
                gmoves.AddMove(EncodeMove(MoveType::PROMOTION, (Square)(x + 8), (Square)x, 0, (uint8_t)ColorPiece::BB));//todo: check
                reset_lsb(quiet_promote);
            }
            while (available2)
            {
                auto x = bit_scan_forward(available2);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x + 7), (Square)x, (uint8_t)piece_board[x]));//todo: check
                reset_lsb(available2);
            }
            while (available1)
            {
                auto x = bit_scan_forward(available1);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x + 9), (Square)x, (uint8_t)piece_board[x]));//todo: check
                reset_lsb(available1);
            }
            while (doublemove)
            {
                auto x = bit_scan_forward(doublemove);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x + 16), (Square)x, 0, 0, (Square)x));//todo: check
                reset_lsb(doublemove);
            }
            while (quiet)
            {
                auto x = bit_scan_forward(quiet);
                gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)(x + 8), (Square)x));//todo: check
                reset_lsb(quiet);
            }
            //en_passant
            //auto en_passant_move = move_history[move_count - 1];
            //Board.print_move(en_passant_move);
            //auto to = (int)GetMoveTo(en_passant_move);
            /*
            if (piece_board[to] == ColorPiece::WP && ((int)GetMoveFrom(en_passant_move) / 8 == 1) && (to / 8 == 3)) {
                auto en_try1 = to - 1;
                auto en_try2 = to + 1;
                if (en_try2 / 8 == 3) if (get_bit(bboard[(int)ColorPiece::BP], (int)en_try2)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try2, (Square)to, (uint8_t)piece_board[to])); }
                if (en_try1 / 8 == 3) if (get_bit(bboard[(int)ColorPiece::BP], (int)en_try1)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try1, (Square)to, (uint8_t)piece_board[to])); }
            }
            */
            if ((int)en_passant/8 == 3) {
                const auto en_try1 = (int)en_passant - 1;
                const auto en_try2 = (int)en_passant + 1;
                //std::cout << en_try1 << ' ' << en_try2 << '\n';
                if (en_try2 / 8 == 3) if (get_bit(bboard[(int)ColorPiece::BP], (int)en_try2)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try2, (Square)en_passant, (uint8_t)piece_board[(int)en_passant])); }
                if (en_try1 / 8 == 3) if (get_bit(bboard[(int)ColorPiece::BP], (int)en_try1)) { gmoves.AddMove(EncodeMove(MoveType::EN_PASSANT, (Square)en_try1, (Square)en_passant, (uint8_t)piece_board[(int)en_passant])); }
            }
            
            //available = (available << 9);
            // 0 0 0   0 0 0   0 0 0
            // 0 1 1 & 0 1 0 = 0 1 0
            // 0 0 0   0 0 0   0 0 0 
        }
        template<bool IsWhite> inline void GenQueenMoves(MoveList& gmoves) {
            bitboard available;
            if constexpr (IsWhite) available = bboard[(int)Piece::Q];
            else available = bboard[(int)ColorPiece::BQ];
            while (available)
            {
                auto pos = bit_scan_forward(available);
                auto movesbb = BishopAttacks(pos, cur_all) | RookAttacks(pos, cur_all);
                movesbb &= not_own;
                while (movesbb)
                {
                    auto x = bit_scan_forward(movesbb);
                    gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)pos, (Square)x, (uint8_t)piece_board[x]));
                    reset_lsb(movesbb);
                }
                reset_lsb(available);
            }
        }
        template<bool IsWhite> inline void GenBishopMoves(MoveList& gmoves) {
            bitboard available;
            if constexpr (IsWhite) available = bboard[(int)Piece::B];
            else available = bboard[(int)ColorPiece::BB];
            while (available)
            {
                auto pos = bit_scan_forward(available);
                auto movesbb = BishopAttacks(pos, cur_all);
                movesbb &= not_own;
                while (movesbb)
                {
                    auto x = bit_scan_forward(movesbb);
                    gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)pos, (Square)x, (uint8_t)piece_board[x]));
                    reset_lsb(movesbb);
                }
                reset_lsb(available);
            }
        }
        template<bool IsWhite> inline void GenRookMoves(MoveList& gmoves) {
            bitboard available;
            if constexpr (IsWhite) available = bboard[(int)Piece::R];
            else available = bboard[(int)ColorPiece::BR];
            while (available)
            {
                auto pos = bit_scan_forward(available);
                auto movesbb = RookAttacks(pos, cur_all);
                movesbb &= not_own;
                while (movesbb)
                {
                    auto x = bit_scan_forward(movesbb);
                    gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)pos, (Square)x, (uint8_t)piece_board[x]));
                    reset_lsb(movesbb);
                }
                reset_lsb(available);
            }
        }
        template<bool IsWhite> inline void GenKingMoves(MoveList& gmoves) {
            bitboard available;
            if constexpr (IsWhite) available = bboard[(int)Piece::K];
            else available = bboard[(int)ColorPiece::BK];
            while (available)
            {
                auto pos = bit_scan_forward(available);
                auto movesbb = KingAttacks(pos);
                movesbb &= not_own;
                while (movesbb)
                {
                    auto x = bit_scan_forward(movesbb);
                    gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)pos, (Square)x, (uint8_t)piece_board[x]));
                    reset_lsb(movesbb);
                }
                reset_lsb(available);
            }
        }
        template<bool IsWhite> inline void GenKnightMoves(MoveList& gmoves) {
            bitboard available;
            if constexpr (IsWhite) available = bboard[(int)Piece::N];
            else available = bboard[(int)ColorPiece::BN];
            while (available)
            {
                auto pos = bit_scan_forward(available);
                auto movesbb = KnightAttacks(pos);
                movesbb &= not_own;
                while (movesbb)
                {
                    auto x = bit_scan_forward(movesbb);
                    gmoves.AddMove(EncodeMove(MoveType::NORMAL, (Square)pos, (Square)x, (uint8_t)piece_board[x]));
                    reset_lsb(movesbb);
                }
                reset_lsb(available);
            }
        }
        template<bool IsWhite> inline void GenCastleQueenside(MoveList& gmoves) {
            if (IsKingInCheck<IsWhite>() || (!CanCastleQueenside<IsWhite>())) return;
            if constexpr (IsWhite) {
                #define temp_use_xx bboard[(int)ColorPiece::WK]
                auto temp = (temp_use_xx >> 1) | (temp_use_xx >> 2) | (temp_use_xx >> 3);
                if ((temp & cur_all)) return;
                temp_use_xx  >>= 1; 
                UpdateOccupancy();
                if (IsKingInCheck<true>()) {temp_use_xx <<= 1;return;}
                temp_use_xx <<= 1;
                gmoves.AddMove(EncodeMove(MoveType::CASTLES, Square::E1, Square::C1));
                #undef temp_use_xx
            } 
            else {
                #define temp_use_xx bboard[(int)ColorPiece::BK]
                auto temp = (temp_use_xx >> 1) | (temp_use_xx >> 2) | (temp_use_xx >> 3);
                if ((temp & cur_all)) return;
                temp_use_xx  >>= 1; 
                UpdateOccupancy();
                if (IsKingInCheck<false>()) {temp_use_xx <<= 1;return;}
                temp_use_xx <<= 1;
                gmoves.AddMove(EncodeMove(MoveType::CASTLES, Square::E8, Square::C8));
                #undef temp_use_xx
            }
        }
        template<bool IsWhite> inline void GenCastleKingside(MoveList& gmoves) {
            if (IsKingInCheck<IsWhite>() || (!CanCastleKingside<IsWhite>())) return;
            if constexpr (IsWhite) {
                #define temp_use_xx bboard[(int)ColorPiece::WK]
                auto temp = (temp_use_xx << 1) | (temp_use_xx << 2);
                if ((temp & cur_all)) return;
                temp_use_xx  <<= 1; 
                UpdateOccupancy();
                if (IsKingInCheck<true>()) {temp_use_xx  >>= 1; return;}
                temp_use_xx  >>= 1; 
                gmoves.AddMove(EncodeMove(MoveType::CASTLES, Square::E1, Square::G1));
                #undef temp_use_xx
            } 
            else {
                #define temp_use_xx bboard[(int)ColorPiece::BK]
                auto temp = (temp_use_xx << 1) | (temp_use_xx << 2);
                if ((temp & cur_all)) return;
                temp_use_xx <<= 1;
                UpdateOccupancy();
                if (IsKingInCheck<false>()) {temp_use_xx  >>= 1; return;}
                temp_use_xx  >>= 1; 
                gmoves.AddMove(EncodeMove(MoveType::CASTLES, Square::E8, Square::G8));
                #undef temp_use_xx
            }
        }
        template<bool IsWhite> inline bool CanCastleQueenside() {if constexpr(IsWhite) return castle & 8;/*0b1000*/else return castle & 2;/*0b0010*/}
        template<bool IsWhite> inline bool CanCastleKingside() {if constexpr(IsWhite) return castle & 4;/*0b0100*/else return castle & 1;/*0b0001*/}
    public:
        // random piece keys [piece][square]
        static inline hash_key piece_keys[12][64] = {0};
        static inline hash_key enpassant_keys[64] = {0};
        static inline hash_key castle_keys[16] = {0};
        static inline hash_key side_key = {0};

        hash_key current_key;
        Square en_passant = Square::A1;
        int castle = 15;//1111
        bitboard bboard[13];
        ColorPiece piece_board[64];
        bitboard cur_all, not_own, enemy;
        Move move_history[1300];
        int move_count = 1;
    public:
        inline static void InitEvalMasks() {
            for (int i = 0; i < 8; i++) {
                isolated_masks[i] = 0;
                if (i != 0) isolated_masks[i] |= GetFileMask(i-1);
                if (i != 7) isolated_masks[i] |= GetFileMask(i+1);
            }
            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    int posw = rank * 8 + file;
                    int pos1 = posw;
                    passed_masks[1][pos1] = 0;
                    posw+=8;
                    while (posw < 64) {
                        if (file!=0) set_bit(passed_masks[1][pos1], (posw-1));
                        if (file!=7) set_bit(passed_masks[1][pos1], (posw+1));
                        set_bit(passed_masks[1][pos1], (posw));
                        posw+=8;
                    }
                    int posb = (7 - rank) * 8 + file;
                    int pos2 = posb;
                    passed_masks[0][pos2] = 0;
                    posb-=8;
                    while (posb >= 0) {
                        if (file!=0) set_bit(passed_masks[0][pos2], (posb-1));
                        if (file!=7) set_bit(passed_masks[0][pos2], (posb+1));
                        set_bit(passed_masks[0][pos2], (posb));
                        posb-=8;
                    }
                }

            }
        }
        static void InitAll() {
            srand(time(NULL));
            for (int i = 0; i < 64; ++i) {
                //pawn_attacks[0][i] = mask_pawn_attacks(0, i);
                //pawn_attacks[1][i] = mask_pawn_attacks(1, i);
                king_attacks[i] = mask_king_attacks(i);
                knight_attacks[i] = mask_knight_attacks(i);
            }
            //hash_table.resize(hash_size);
            //hash_table.resize(hash_size);
            InitSliders(1);InitSliders(0);
            for (int i = 0; i< 64; ++i) {
                for (int j = 0; j < 12; ++j) {
                    piece_keys[j][i] = get_random_U64_number();
                }
                enpassant_keys[i] = get_random_U64_number();
            }
            for (int i = 0; i < 16; ++i) castle_keys[i] = get_random_U64_number();
            side_key = get_random_U64_number();
            InitEvalMasks();
        }
        static inline bitboard GetFileMask(int file) {return file_masks[file];}
        static inline bitboard GetRankMask(int rank) {return rank_masks[rank];}
        static inline bitboard GetIsolatedMask(int file) {return isolated_masks[file];}

        template<bool IsWhite> static inline bitboard GetPassedMaskSquare(int square) {return passed_masks[IsWhite][square];}
        static inline bitboard GetIsolatedMaskSquare(int square) {return isolated_masks[square%8];}
        static inline bitboard GetFileMaskSquare(int square) {return file_masks[square%8];}
        static inline bitboard GetRankMaskSquare(int square) {return rank_masks[square/8];}
        static inline bitboard BishopAttacks(int square, bitboard occupancy) {return bishop_attacks[square][((occupancy & bishop_masks[square]) * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square])];}
        static inline bitboard RookAttacks(int square, bitboard occupancy)
        {
            occupancy &= rook_masks[square];
            occupancy *= rook_magic_numbers[square];
            occupancy >>= 64 - rook_relevant_bits[square];
            return rook_attacks[square][occupancy];
        }
        static inline bitboard QueenAttacks(int square, bitboard occupancy) {return RookAttacks(square, occupancy) | BishopAttacks(square, occupancy);}
        static inline bitboard KnightAttacks(int square) {return knight_attacks[square];}
        static inline bitboard KingAttacks(int square) { return king_attacks[square]; }
        template<bool IsWhite> static inline bitboard PawnAttacks(int square) {return pawn_attacks[IsWhite][square];}
    private:
        static bitboard mask_knight_attacks(int square)
        {
            bitboard bbitboard = 0;
            constexpr int offsetsx[] = {2, -2, 2, -2, 1, -1, 1, -1};
            constexpr int offsetsy[] = {1, 1, -1, -1, 2, 2, -2, -2};
            int sqx = square % 8;
            int sqy = square / 8;
            for (int i= 0;i<8;++i) 
            {
                auto newx = sqx + offsetsx[i];
                auto newy = sqy + offsetsy[i];
                if (newx >= 0 && newy <8 && newy >= 0 && newx <8) {set_bit(bbitboard, newy*8+newx);}
            }
            return bbitboard;
        }
        static bitboard mask_king_attacks(int square) {
            bitboard bb=0;
            constexpr int offsetx[] = {1, 1, 1, 0, 0, -1, -1, -1};
            constexpr int offsety[] = {0, 1, -1, 1, -1, 1, 0, -1};
            int sqx = square % 8;
            int sqy = square / 8;
            for (int i= 0;i<8;++i) 
            {
                auto newx = sqx + offsetx[i];
                auto newy = sqy + offsety[i];
                if (newx >= 0 && newy <8 && newy >= 0 && newx <8) {set_bit(bb, newy*8+newx);}
            }
            return bb;
        }
        static bitboard mask_bishop_attacks(int square)
        {
            bitboard attacks = 0ULL;
            int r, f;
            int tr = square / 8;
            int tf = square % 8;
            for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
            for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
            for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
            for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
            return attacks;
        }
        static bitboard mask_rook_attacks(int square)
        {
            bitboard attacks = 0ULL;
            int r, f;
            int tr = square / 8;
            int tf = square % 8;
            for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
            for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
            for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
            for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
            return attacks;
        }
        static bitboard bishop_attacks_on_the_fly(int square, bitboard block)
        {
            bitboard attacks = 0ULL;
            int r, f;
            int tr = square / 8;
            int tf = square % 8;
            for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
            {
                attacks |= (1ULL << (r * 8 + f));
                if ((1ULL << (r * 8 + f)) & block) break;
            }
            
            for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
            {
                attacks |= (1ULL << (r * 8 + f));
                if ((1ULL << (r * 8 + f)) & block) break;
            }
            
            for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
            {
                attacks |= (1ULL << (r * 8 + f));
                if ((1ULL << (r * 8 + f)) & block) break;
            }
            
            for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
            {
                attacks |= (1ULL << (r * 8 + f));
                if ((1ULL << (r * 8 + f)) & block) break;
            }
            return attacks;
        }
        static bitboard rook_attacks_on_the_fly(int square, bitboard block)
        {
            bitboard attacks = 0ULL;
            int r, f;
            int tr = square / 8;
            int tf = square % 8;
            for (r = tr + 1; r <= 7; r++)
            {
                attacks |= (1ULL << (r * 8 + tf));
                if ((1ULL << (r * 8 + tf)) & block) break;
            }
            for (r = tr - 1; r >= 0; r--)
            {
                attacks |= (1ULL << (r * 8 + tf));
                if ((1ULL << (r * 8 + tf)) & block) break;
            }
            for (f = tf + 1; f <= 7; f++)
            {
                attacks |= (1ULL << (tr * 8 + f));
                if ((1ULL << (tr * 8 + f)) & block) break;
            }
            for (f = tf - 1; f >= 0; f--)
            {
                attacks |= (1ULL << (tr * 8 + f));
                if ((1ULL << (tr * 8 + f)) & block) break;
            }
            return attacks;
        }
        static void InitSliders(int bishop)
        {
            auto set_occupancy = [](int index, int bits_in_mask, bitboard attack_mask)
            {
                bitboard occupancy = 0ULL;
                for (int count = 0; count < bits_in_mask; count++)
                {
                    int square = bit_scan_forward(attack_mask);
                    pop_bit(attack_mask, square);
                    
                    if (index & (1 << count))
                        occupancy |= (1ULL << square);
                }
                return occupancy;
            };
            for (int square = 0; square < 64; square++)
            {
                bishop_masks[square] = mask_bishop_attacks(square);
                rook_masks[square] = mask_rook_attacks(square);
                bitboard attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
                int relevant_bits_count = count_bits(attack_mask);
                int occupancy_indicies = (1 << relevant_bits_count);
                for (int index = 0; index < occupancy_indicies; index++)
                {
                    if (bishop)
                    {
                        bitboard occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
                    }
                    else
                    {
                        bitboard occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
                    
                    }
                }
            }
        } 
        // castling rights update constants
        inline static constexpr int castling_rights[64] = {
            7, 15, 15, 15,  3, 15, 15, 11,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            13, 15, 15, 15, 12, 15, 15, 14
        };
        inline static bitboard pawn_attacks[2][64] = {};
        inline static bitboard knight_attacks[64] = {};
        inline static bitboard king_attacks[64] = {};
        inline static bitboard bishop_masks[64] = {};
        inline static bitboard rook_masks[64] = {};
        // bishop attacks table [square][occupancies]
        inline static bitboard bishop_attacks[64][512] = {};
        // rook attacks rable [square][occupancies]
        inline static bitboard rook_attacks[64][4096] = {};

        static constexpr inline bitboard file_masks[8] = {0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808, 0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080};
        static constexpr inline bitboard rank_masks[8] = {0xFF, 0xFF00, 0xFF0000, 0xFF000000, 0xFF00000000, 0xFF0000000000, 0xFF000000000000, 0xFF00000000000000};
        static inline bitboard isolated_masks[8];
        static inline bitboard passed_masks[2][64];


        static constexpr bitboard rook_magic_numbers[64] = {0x8a80104000800020ULL,0x140002000100040ULL,0x2801880a0017001ULL,0x100081001000420ULL,0x200020010080420ULL,0x3001c0002010008ULL,0x8480008002000100ULL,0x2080088004402900ULL,0x800098204000ULL,0x2024401000200040ULL,0x100802000801000ULL,0x120800800801000ULL,0x208808088000400ULL,0x2802200800400ULL,0x2200800100020080ULL,0x801000060821100ULL,0x80044006422000ULL,0x100808020004000ULL,0x12108a0010204200ULL,0x140848010000802ULL,0x481828014002800ULL,0x8094004002004100ULL,0x4010040010010802ULL,0x20008806104ULL,0x100400080208000ULL,0x2040002120081000ULL,0x21200680100081ULL,0x20100080080080ULL,0x2000a00200410ULL,0x20080800400ULL,0x80088400100102ULL,0x80004600042881ULL,0x4040008040800020ULL,0x440003000200801ULL,0x4200011004500ULL,0x188020010100100ULL,0x14800401802800ULL,0x2080040080800200ULL,0x124080204001001ULL,0x200046502000484ULL,0x480400080088020ULL,0x1000422010034000ULL,0x30200100110040ULL,0x100021010009ULL,0x2002080100110004ULL,0x202008004008002ULL,0x20020004010100ULL,0x2048440040820001ULL,0x101002200408200ULL,0x40802000401080ULL,0x4008142004410100ULL,0x2060820c0120200ULL,0x1001004080100ULL,0x20c020080040080ULL,0x2935610830022400ULL,0x44440041009200ULL,0x280001040802101ULL,0x2100190040002085ULL,0x80c0084100102001ULL,0x4024081001000421ULL,0x20030a0244872ULL,0x12001008414402ULL,0x2006104900a0804ULL,0x1004081002402ULL};
        static constexpr bitboard bishop_magic_numbers[64] = {0x40040844404084ULL,0x2004208a004208ULL,0x10190041080202ULL,0x108060845042010ULL,0x581104180800210ULL,0x2112080446200010ULL,0x1080820820060210ULL,0x3c0808410220200ULL,0x4050404440404ULL,0x21001420088ULL,0x24d0080801082102ULL,0x1020a0a020400ULL,0x40308200402ULL,0x4011002100800ULL,0x401484104104005ULL,0x801010402020200ULL,0x400210c3880100ULL,0x404022024108200ULL,0x810018200204102ULL,0x4002801a02003ULL,0x85040820080400ULL,0x810102c808880400ULL,0xe900410884800ULL,0x8002020480840102ULL,0x220200865090201ULL,0x2010100a02021202ULL,0x152048408022401ULL,0x20080002081110ULL,0x4001001021004000ULL,0x800040400a011002ULL,0xe4004081011002ULL,0x1c004001012080ULL,0x8004200962a00220ULL,0x8422100208500202ULL,0x2000402200300c08ULL,0x8646020080080080ULL,0x80020a0200100808ULL,0x2010004880111000ULL,0x623000a080011400ULL,0x42008c0340209202ULL,0x209188240001000ULL,0x400408a884001800ULL,0x110400a6080400ULL,0x1840060a44020800ULL,0x90080104000041ULL,0x201011000808101ULL,0x1a2208080504f080ULL,0x8012020600211212ULL,0x500861011240000ULL,0x180806108200800ULL,0x4000020e01040044ULL,0x300000261044000aULL,0x802241102020002ULL,0x20906061210001ULL,0x5a84841004010310ULL,0x4010801011c04ULL,0xa010109502200ULL,0x4a02012000ULL,0x500201010098b028ULL,0x8040002811040900ULL,0x28000010020204ULL,0x6000020202d0240ULL,0x8918844842082200ULL,0x4010011029020020ULL};
        static constexpr int bishop_relevant_bits[64] = {6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};
        static constexpr int rook_relevant_bits[64] = {12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};
    };
    namespace UCI{
        inline bool uci_debug = false;
        inline int starttime, stoptime;
        inline std::atomic_bool stopped;
    }
    /*
        Move ordering
        * PV moves
        * Captures in mvv/lva order
        * 2 Killers
        * History 
    */
   /*
        Search
        * todo: write search info
   */
    struct SearchEngine{
    public:
        static constexpr int EVAL_DRAW = 0;
        static constexpr int EVAL_MATE = 32000;
        static constexpr int EVAL_INFINITY = 33000;
        static constexpr int VeryLateMove = 13;
        static constexpr int VeryLatePly = 3;
        static constexpr int FullDepthMoves = 4;
        static constexpr int WindowMargin = 50;
        static constexpr int FutilityMargin = 50;
        static constexpr bool UseFutilityPruning = false;
        static constexpr bool UseReverseFutilityPruning = true;
        //maybe internal iterative deepening like in  fruit
        static const int IIDepth = 3;
        bool use_book = false, follow_pv, do_null_move = true;
        int ply=0;
        Board board;
        int nodes = 0;
        int stoptime;
        bool look_at_book = true;
        std::unordered_multimap<std::string, std::string> book_moves;
        template<bool IsWhite> uint64_t perft(int depth) {
            if (depth == 0) return 1;
            uint64_t nodes = 0;
            MoveList moves;
            board.GenMovesUnchecked<IsWhite>(moves);
            for (int i = 0; i < moves.count; ++i) {
                board.MakeMove<IsWhite>(moves.move_storage[i]);
                if (board.IsKingInCheck<IsWhite>()) {
                    board.UndoMove<IsWhite>(moves.move_storage[i]);
                    continue;
                }
                nodes+=perft<!IsWhite>(depth-1);
                board.UndoMove<IsWhite>(moves.move_storage[i]);
            }
            return nodes;
        }
        template<bool IsWhite> void perft_divide(int depth) {
            if (depth == 0) return;
            uint64_t nodes = 0;
            MoveList moves;
            board.GenMovesUnchecked<IsWhite>(moves);
            for (int i = 0; i < moves.count; ++i) {
                board.MakeMove<IsWhite>(moves.move_storage[i]);
                if (board.IsKingInCheck<IsWhite>()) {
                    board.UndoMove<IsWhite>(moves.move_storage[i]);
                    continue;
                }
                const auto x = perft<!IsWhite>(depth-1);
                std::cout << move_to_string(moves.move_storage[i]) << " : " << x << '\n';
                nodes+=x;
                board.UndoMove<IsWhite>(moves.move_storage[i]);
            }
            std::cout << "Total: " << nodes << '\n';
        }void init_book(std::string path) {
            std::ifstream book;
            book.open(path);
            if (book.fail() || !book.is_open()) {
                use_book = false;
                book_moves.clear();
                std::cout << "Book not found!" << std::endl;
                return;
            }
            int curLine = 0;
            std::string line = "";
            std::string move_str = board.GetFen();
            move_str = move_str.substr(0, move_str.size()-4);
            while(getline(book, line)) {
                curLine++;
                const std::string fen = line.substr(0, line.find('=')-4);
                const std::string move_data = line.substr(line.find("=>")+3);
                std::vector<std::string> data;
                split(move_data, ' ', data);
                for (size_t i = 0; i < data.size(); ++i) {book_moves.insert({fen, data[i]});}
            }
        }
        template <bool IsWhite, bool HasTime>
        void go_position(int depth, int endtime = 0,__attribute__((unused))  bool very_fast = false) {
            int alpha = -EVAL_INFINITY, beta = EVAL_INFINITY, score=0;
            if constexpr (HasTime) stoptime = endtime;
            hash_hit = 0;
            do_null_move = (count_bits(board.bboard[(int)ColorPiece::WP])*100 + count_bits(board.bboard[(int)ColorPiece::BP]*100)
             + count_bits(board.bboard[(int)ColorPiece::WN])*300 + count_bits(board.bboard[(int)ColorPiece::BN]*300)
             + count_bits(board.bboard[(int)ColorPiece::WB])*300 + count_bits(board.bboard[(int)ColorPiece::BB] * 300)
             + count_bits(board.bboard[(int)ColorPiece::WR])*500 + count_bits(board.bboard[(int)ColorPiece::BR]*500)
             + count_bits(board.bboard[(int)ColorPiece::WQ])*910 + count_bits(board.bboard[(int)ColorPiece::BQ])*910) > 2800;
            //std::cout << stoptime << ' ' << get_time_ms() << '\n';
            ply = 0; nodes = 0;follow_pv = false;
            memset(killers, 0, sizeof killers);
            memset(history, 0, sizeof history);
            memset(pv_table, 0, sizeof pv_table);
            memset(pv_length, 0, sizeof pv_length);
            #ifndef TEST_RELEASE
            if (use_book && look_at_book) {
                auto range = book_moves.equal_range(board.GetFen());
                const int rsize = std::distance(range.first, range.second);
                int k = 0;
                srand(time(NULL));
                if (rsize) {
                    const int el = rand()%rsize;
                    for (auto i = range.first; i != range.second; ++i, ++k) {
                        if (k == el) {
                            if (UCI::uci_debug) std::cout << "info string book move\n";
                            std::cout << "bestmove " << i->second << '\n';
                            std::cout.flush();
                            UCI::stopped = true;
                            return;
                        }
                    }
                }
                look_at_book=false;
            }
            #endif
            for (int i = 1; i <= depth; ++i) {
                follow_pv = true;
                //std::cout << "info depth " << i << '\n';
                score = negamax<IsWhite, HasTime>(i, alpha, beta);
                if (UCI::stopped) break;
                if constexpr (HasTime){
                    if (get_time_ms() > stoptime) {
                        UCI::stopped = true;
                        break;
                    }
                } 
                if (score<=alpha || score>=beta) {
                    alpha = -EVAL_INFINITY;
                    beta = EVAL_INFINITY;
                    if (UCI::uci_debug) std::cout << "info depth " << i << " nodes " << nodes << " string fell outside of aspiration window\n";
                    continue;
                }
                alpha= score - WindowMargin;
                beta = score + WindowMargin;
                if (pv_length[0]) {
                    if (abs(abs(score) - EVAL_MATE) < MAX_PLY) {
                        if (score < 0) std::cout << "info depth " << i << " nodes " << nodes << " score mate " << (-score - EVAL_MATE - 1) / 2 << " pv ";
                        else std::cout << "info depth " << i << " nodes " << nodes << " score mate " << (EVAL_MATE - score + 1) / 2 << " pv ";
                    }
                    else std::cout << "info depth " << i << " nodes " << nodes << " score cp " << score << " pv ";
                    for (int i = 0; i < pv_length[0]; ++i) {
                        std::cout << move_to_string(pv_table[0][i]) << ' ';
                    }
                    std::cout << '\n';
                }else {
                    std::cout << "info depth " << i << " nodes " << nodes << " score cp " << score << "\n";
                }
                std::cout.flush();
            }
            std::cout << "info string hash_hit = " << hash_hit << '\n';
            std::cout << "bestmove " << move_to_string(pv_table[0][0]) << '\n';
            std::cout.flush();
            UCI::stopped = true;
        }
        int score_move(const Move& move) {
            if (GetMoveCapture(move)) return mvv_lva[(int)board.piece_board[(int)GetMoveFrom(move)]-1][(int)board.piece_board[(int)GetMoveTo(move)]-1] + 10000 + ((GetMoveType(move) == MoveType::PROMOTION) ? promotion_score[(int)GetMoveParam(move)] : 0);
            if (GetMoveType(move) == MoveType::PROMOTION) return promotion_score[(int)GetMoveParam(move)];
            if (killers[0][ply] == move) return 9000;
            if (killers[1][ply] == move) return 8000;
            return history[(int)board.piece_board[(int)GetMoveFrom(move)]-1][(int)GetMoveTo(move)];
        }
        void sort_moves(MoveList& moves, int offset) {
            std::sort(moves.move_storage + offset, moves.move_storage+moves.count, [&](const Move& left, const Move& right){
                return score_move(left) > score_move(right);
            });
        }
        int find_best(MoveList& moves, Move best, int pos) {
            for (int i = pos; i < moves.count; ++i) {
                if (moves.move_storage[i] == best) {
                    std::swap(moves.move_storage[pos], moves.move_storage[i]);
                    return 1;
                }
            }
            return 0;
        }
        int enable_pvs(MoveList& moves) {
            follow_pv = false;
            for (int i = 0; i < moves.count; ++i) {
                if ((uint32_t)pv_table[0][ply] == moves.move_storage[i]) {
                    follow_pv=true;
                    //if (UCI::uci_debug) std::cout << "info string following principle variation...\n";
                    std::swap(moves.move_storage[0], moves.move_storage[i]);
                    return 1;
                }
            }
            return 0;
        }
        //
        void QuickSort(__attribute__((unused)) int l, __attribute__((unused)) int r, std::vector<int>& scores, MoveList& moves) {
            for (int i = 0; i < (int)scores.size(); i++) {
                for (int j = i + 1; j < (int)scores.size(); ++j) {
                    if (scores[i] < scores[j]) {
                        std::swap(scores[i], scores[j]);
                        std::swap(moves.move_storage[i], moves.move_storage[j]);
                    }
                }
            }
        }

        template<bool IsWhite, bool HasTime, bool Eval=false>
        int quiet_search(int alpha, int beta) {
            nodes++;
            if (is_repetition(board.current_key)) return EVAL_DRAW;
            auto eval = Evaluate<IsWhite>();
            if (eval >= beta) return eval;
            if (eval > alpha) alpha = eval;
            if (ply > MAX_PLY-1) return eval;
            MoveList moves;
            board.GenMovesUnchecked<IsWhite>(moves);
            sort_moves(moves, 0);
            for (int i = 0; i < moves.count;++i) {
                if (!GetMoveCapture(moves.move_storage[i])) continue;
                auto move = moves.move_storage[i];
                board.MakeMove<IsWhite>(move);
                if (board.IsKingInCheck<IsWhite>()) {board.UndoMove<IsWhite>(move);continue;}
                ++ply;
                repetition_table[++repetition_index] = board.current_key;
                //std::cout << spaces << "Quiet search move: " << move_to_string(move) << '\n';
                int score = -quiet_search<!IsWhite, HasTime>(-beta, -alpha);
                --ply;
                --repetition_index;
                //std::cout << spaces << move_to_string(move) << " : " << score << '\n';
                board.UndoMove<IsWhite>(move);
                if (score > alpha) {
                    alpha = score;
                    if (score >= beta) {
                        return score;
                    }
                }
            }
            //std::cout << spaces << "Is White to move? " << IsWhite << '\n';
            //std::cout << spaces << "Best found: " << move_to_string(best) << '\n';
            //std::cout << spaces << "Returining score: " << alpha << '\n';
            //std::cout << spaces << "Move count: " << temp_move_count << '\n';
            //std::cout << spaces << "In Fen: " << board.GetFen() << " static eval: " << Evaluate<IsWhite>() << '\n';
            return alpha;
        }
        inline static constexpr bool is_dangerous(const Move& move, const bool& in_check) {
            return in_check || GetMoveCapture(move) || (GetMoveType(move) == MoveType::PROMOTION);
        }
        template <bool IsWhite, bool HasTime>
        int negamax(int depth, int alpha, int beta, bool can_null_move = true) {
            ++nodes;
            pv_length[ply] = ply;
            int score;
            Move best = NullMove;
            if (ply && is_repetition(board.current_key)) return EVAL_DRAW;
            bool pv_node = beta-alpha > 1;
            if (ply && (score = read_hash_entry(depth, board.current_key, alpha, beta, best)) != NO_HASH && !pv_node) {
                return score;
            }
            if (depth <= 0) return quiet_search<IsWhite, HasTime>(alpha, beta);
            if (ply > MAX_PLY-1) return Evaluate<IsWhite>();
            bool in_check = board.IsKingInCheck<IsWhite>();
            bool f_prune = false; 
            int eval = -EVAL_INFINITY;
            if (in_check) depth++;
            else if (!pv_node) {
                //static eval pruning ~30 elo
                if (UseReverseFutilityPruning && depth < 3) //abs(beta - 1) > -EVAL_INFINITY + 100 ??????????
                {
                    eval = Evaluate<IsWhite>();
                    const int eval_margin = 120 * depth;
                    const int diff = eval - eval_margin;
                    if (diff >= beta) return diff;
                }
                //null move pruning ~ 100 elo
                if (do_null_move && can_null_move && depth>=3 && ply) {
                    const auto saved_pv = follow_pv;
                    //const auto saved_key = board.current_key;
                    board.move_count++;ply++;repetition_table[++repetition_index] = board.current_key;
                    board.current_key^=board.side_key;
                    if ((int)board.en_passant) {
                        const int saved = (int)board.en_passant;
                        board.en_passant = Square::A1;
                        board.current_key^=Board::enpassant_keys[saved];
                        board.current_key^=Board::enpassant_keys[0];
                        
                        score = -negamax<!IsWhite, HasTime>(depth - 1 - 2, -beta, -beta+1, false);
                        
                        board.en_passant = (Square)saved;
                        board.current_key^=Board::enpassant_keys[0];
                        board.current_key^=Board::enpassant_keys[saved];
                    }
                    else {
                        score = -negamax<!IsWhite, HasTime>(depth - 1 - 2, -beta, -beta+1, false);
                    }
                    ply--;--repetition_index;
                    board.move_count--;
                    board.current_key^=board.side_key;
                    follow_pv = saved_pv;
		            //do_null_move=true;
                    if (UCI::stopped) return 0;
                    if constexpr (HasTime) {if (get_time_ms() > stoptime) {UCI::stopped = true;return 0;}}
                    if (score >= beta) {
                        return score;
                    }
                }
                //futility pruning ~0 elo
                if (UseFutilityPruning && depth == 1 && abs(alpha) < 9000) {
                    if (eval == -EVAL_INFINITY) eval = Evaluate<IsWhite>();
                    if (eval + FutilityMargin <= alpha) f_prune = true;
                }
               
            }
            int hashf = HASHF_ALPHA;
            int temp_move_count = 0;
            MoveList moves;
            board.GenMovesUnchecked<IsWhite>(moves);
            int temp_pos=0;
            if (follow_pv) temp_pos = enable_pvs(moves);
            if (best) temp_pos += find_best(moves, best, temp_pos);
            sort_moves(moves, temp_pos);
            
            //for (int i = 1; i < moves.count; ++i) {assert(scores[i-1] <= scores[i]);}
            for (int i = 0; i < moves.count; ++i) {
            //for (int i = moves.count-1; i >= 0; --i) {
                auto move = moves.move_storage[i];
                board.MakeMove<IsWhite>(move);
                if (board.IsKingInCheck<IsWhite>()) {board.UndoMove<IsWhite>(move);continue;}
                if (f_prune && !is_dangerous(move, in_check) && killers[0][ply] != moves.move_storage[i] && killers[0][ply] != moves.move_storage[i]) {board.UndoMove<IsWhite>(move);break;}
                ++ply;repetition_table[++repetition_index] = board.current_key;
                if (!temp_move_count) score = -negamax<!IsWhite, HasTime>(depth-1, -beta, -alpha);
                //boring move check
                
                else {
                    int red = 0;
                    if (!is_dangerous(move, in_check)) {
                        //Late Move Reduction(LMR)
                        if (temp_move_count>=FullDepthMoves && depth >= 3) {
                            red = lmred[pv_node][std::min(depth, 32)][std::min(63, temp_move_count)]; 
                            score = -negamax<!IsWhite, HasTime>(depth - 1 - red, -alpha-1, -alpha);
                        }
                        else score = alpha+1;
                    } 
                    /*
                    if (ply && (!in_check) &&
                    (!GetMoveCapture(move)) && 
                    (GetMoveType(move) != MoveType::PROMOTION)) {
                        if (depth > ReductionLimit && temp_move_count > FullDepthMoves) score = -negamax<!IsWhite, HasTime>(depth-ReductionLimit, -alpha -1, -alpha);
                        //if (depth > ReductionLimit && temp_move_count >= VeryLateMove) score = -negamax<!IsWhite, HasTime>(depth-ReductionLimit, -alpha -1, -alpha);
                        //else if (temp_move_count >= FullDepthMoves && depth >= ReductionLimit) score = -negamax<!IsWhite, HasTime>(depth-ReductionFactor, -alpha -1, -alpha);
                        else score = alpha+1;
                    }*/
                    else score = alpha+1;
                    if (score > alpha) {
                        score = -negamax<!IsWhite, HasTime>(depth - 1, -alpha-1, -alpha);
                        /*if (red == 0) {
                            red = temp_move_count/12 + temp_move_count/16;
                            score = -negamax<!IsWhite, HasTime>(depth - 1 - red, -alpha-1, -alpha);
                            if (red != 0 && score > alpha) score = -negamax<!IsWhite, HasTime>(depth - 1, -alpha-1, -alpha);
                        }
                        else score = -negamax<!IsWhite, HasTime>(depth - 1, -alpha-1, -alpha);*/
                        //search trying to proove that other moves are not better
                        //if we found a better move
                        if ((score > alpha) && (score < beta)) 
                            //research normally
                            score = -negamax<!IsWhite, HasTime>(depth - 1, -beta, -alpha);
                    }
                }
                --ply;--repetition_index;
                board.UndoMove<IsWhite>(move);
                
                ++temp_move_count;
                //1717016
                //R4r1k/6pp/2pq4/2n2b2/2Q1pP1b/1r2P2B/NP5P/2B2KNR b - - 1 24
                //1012458 => 980435
                //8145000 => 5923000
                
                if (UCI::stopped) return 0;
                if constexpr (HasTime) {if (get_time_ms() > stoptime) {UCI::stopped = true;return 0;}}
                if (score >= beta) {
                    if (!GetMoveCapture(move)) {
                        killers[1][ply] = killers[0][ply];
                        killers[0][ply] = moves.move_storage[i];
                        //if (!GetMoveCapture(move)) history[(int)board.piece_board[(int)GetMoveFrom(move)]-1][(int)GetMoveTo(move)] += depth*depth;
                    }
                    write_hash_entry(depth, board.current_key, beta, HASHF_BETA, moves.move_storage[i]);
                    return score;
                }
                if (score > alpha) {
                    if (!GetMoveCapture(move)) history[(int)board.piece_board[(int)GetMoveFrom(move)]-1][(int)GetMoveTo(move)] += depth;
                    best = moves.move_storage[i];
                    
                    alpha = score;
                    hashf = HASHF_EXACT;

                    // write PV move
                    pv_table[ply][ply] = moves.move_storage[i];
                    // loop over the next ply
                    for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                        // copy move from deeper ply into a current ply's line
                        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
                    // adjust PV length
                    pv_length[ply] = pv_length[ply + 1];
                }
            }
            if (temp_move_count == 0) {
                if (board.IsKingInCheck<IsWhite>()) {
                    return -EVAL_MATE + ply;
                }
                return EVAL_DRAW;
            }
            write_hash_entry(depth, board.current_key, alpha, hashf, best);
            return alpha;
        }
        void Init() {
            for (int i = 0; i < 64; ++i) {
                for (int j = 0; j < 64; ++j) {
                    int row_i = i / 8;
                    int file_i = i % 8;
                    int row_j = j / 8;
                    int file_j = j % 8;
                    square_proximity[i][j] = 7 - std::max(std::abs(file_i - file_j), std::abs(row_i - row_j));
                }
            }
            for (int d = 3; d < 33; d++) {
                for (int l = 1; l < 64; l++) {
                    //0 - any node
                    //1 - pv node
                    //lmred[0][d][l] = int(std::log(l) * std::log(d) / 1.95);    //Stockfish
                    //lmred[1][d][l] = int(std::log(l) * std::log(d) / 1.95) - 2;//Stockfish
                    //lmred[0][d][l] = int(sqrtl(d-1) + sqrt(l-1));          //Fruit Reloaded
                    //lmred[1][d][l] = int((sqrtl(d-1) + sqrt(l-1)) * 2 / 3);//Fruit Reloaded

                    //lmred[0][d][l] = int(sqrtl(d-1) + sqrt(l-1));
                    //lmred[1][d][l] = int((sqrtl(d-1) + sqrt(l-1)) * 29 / 45);
                    
                    lmred[0][d][l] = int(std::log2(l) * std::log2(d) * 0.4);//0.4 idealy
                    if (lmred[0][d][l] > d-2) lmred[0][d][l] = d-2;
                    lmred[1][d][l] = int(std::log2(l) * std::log2(d) * 0.2);
                }
            }
            board.InitBoard();
            init_book("simple.book");
        }
    public:
    #define VICE_EXPERIMENTAL
    #if defined(GUNBORG_EVAL)
        static constexpr int flip[64] = {
            56,  57,  58,  59,  60,  61,  62,  63,
            48,  49,  50,  51,  52,  53,  54,  55,
            40,  41,  42,  43,  44,  45,  46,  47,
            32,  33,  34,  35,  36,  37,  38,  39,
            24,  25,  26,  27,  28,  29,  30,  31,
            16,  17,  18,  19,  20,  21,  22,  23,
            8,   9,  10,  11,  12,  13,  14,  15,
            0,   1,   2,   3,   4,   5,   6,   7
        };
        const int DOUBLED_PAWN_PENALTY = 10;//originally 14, now 10
        const int ISOLATED_PAWN_PENALTY = 9;
        const int BACKWARD_PAWN_PENALTY = 7;

        const int PASSED_PAWN_BONUS[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };//const int PASSED_PAWN_BONUS = 38;
        const int OPEN_FILE_BONUS = 10;
        const int SEMI_OPEN_FILE_BONUS = 5;
        const int UNSAFE_KING_PENALTY = 9;
        const int BISHOP_PAIR_BONUS = 44;
        const int BISHOP_MOBILITY_BONUS = 2;
        const int ROOK_MOBILITY_BONUS = 4;
        const int KNIGHT_KING_PROXIMITY_BONUS = 6;
        const int ROOK_KING_PROXIMITY_BONUS = 4;
        const int QUEEN_KING_PROXIMITY_BONUS = 11;
        const int BISHOP_KING_PROXIMITY_BONUS = 0;
        const int MAX_MATERIAL = 3100;

        const int PIECE_VALUES[7] = {100, 300, 300, 500, 900, 10000, 100};

        // the value of a white pawn at all squares from white's perspective
        const int PAWN_SQUARE_TABLE[64] = {
                // A1
                100, 100, 100, 100, 100, 100, 100, 100, // H1
                100, 100, 100, 95, 95, 100, 100, 100,
                101, 103, 104, 98, 98, 104, 103, 101,
                102, 105, 106, 111, 111, 106, 105, 102,
                103, 107, 109, 111, 111, 109, 107, 103,
                105, 109, 111, 111, 111, 111, 109, 105,
                107, 111, 113, 113,	113, 113, 111, 107,
        /*A8*/100, 100, 100, 100, 100, 100, 100, 100 // H8
                };

        const int PAWN_SQUARE_TABLE_ENDGAME[64] = {
                // A1
                100, 100, 100, 100, 100, 100, 100, 100, // H1
                100, 100, 100, 100, 100, 100, 100, 100,
                105, 105, 105, 105, 105, 105, 105, 105,
                111, 111, 111, 111, 111, 111, 111, 111,
                130, 130, 130, 130, 130, 130, 130, 130,
                170, 170, 170, 170, 170, 170, 170, 170,
                220, 220, 220, 220, 220, 220, 220, 220,
        /*A8*/100, 100, 100, 100, 100, 100, 100, 100 // H8
                };

        const int KNIGHT_SQUARE_TABLE[64] = {
                290, 295, 295, 295, 295, 295, 295, 290,
                295, 300, 300, 300, 300, 300, 300, 295,
                295, 300, 305, 307, 307, 305, 300, 295,
                295, 300, 309, 310, 310, 309, 300, 295,
                295, 300, 309, 310, 310, 309, 300, 295,
                295, 300, 305, 307, 307, 305, 300, 295,
                295, 300, 300, 300, 300, 300, 300, 295,
                290, 295, 295, 295, 295, 295, 295, 290, };

        const int BISHOP_SQUARE_TABLE[64] = {
                275, 295, 295, 295, 295, 295, 295, 275,
                295, 300, 300, 300, 300, 300, 300, 295,
                295, 300, 308, 306, 306, 308, 300, 295,
                295, 300, 310, 309, 309, 310, 300, 295,
                295, 300, 311, 310, 310, 311, 300, 295,
                295, 300, 312, 312, 312, 312, 300, 295,
                295, 305, 314, 315, 315, 314, 305, 295,
                295, 305, 305, 309, 309, 305, 305, 295, };

        const int ROOK_SQUARE_TABLE[64] = {
                // A1
                495, 500, 500, 510, 510, 500, 500, 495,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                515, 515, 515, 515, 515, 515, 515, 515,
                495, 500, 500, 500, 500, 500, 500, 495, }; //H8

        const int QUEEN_SQUARE_TABLE[64] {
                895, 895, 895, 895, 895, 895, 895, 895,
                895, 900, 900, 900, 900, 900, 900, 895,
                895, 900, 900, 900, 900, 900, 900, 895,
                895, 900, 900, 905, 905, 900, 900, 895,
                895, 900, 900, 905, 905, 900, 900, 895,
                895, 900, 900, 900, 900, 900, 900, 895,
                895, 900, 900, 900, 900, 900, 900, 895,
                895, 895, 895, 895, 895, 895, 895, 895,
        };

        const int KING_SQUARE_TABLE[64] {
                // A1
                10000, 10010, 10020, 9990, 9995, 9990, 10020, 10000,
                10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
                10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
                10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
                10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
                10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
                10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
                10000, 10010, 10020, 9990, 9995, 9990, 10020, 10000, // H8
        };

        const int KING_SQUARE_TABLE_ENDGAME[64] = {
                9980, 9990, 9990, 9990, 9990, 9990, 9990, 9980,
                9990, 9995, 9995, 9995, 9995, 9995, 9995, 9990,
                9995, 10000, 10000, 10005, 10005, 10000, 10000, 9995,
                9995, 10005, 10005, 10010, 10010, 10005, 10005, 9995,
                9995, 10005, 10005, 10010, 10010, 10005, 10005, 9995,
                9995, 10000, 10000, 10005, 10005, 10000, 10000, 9995,
                9990, 9995, 9995, 9995, 9995, 9995, 9995, 9990,
                9980, 9990, 9990, 9990, 9990, 9990, 9990, 9980
        };
        template<bool IsWhite>
    int Evaluate() {
        //add queen as open and semi open file piece??????
        board.UpdateOccupanciesColors<IsWhite>();
        int score = 0;
        int op_score = 0, eg_score = 0;
	    int black_king_proximity_bonus = 0;
        int white_king_proximity_bonus = 0;
        int bishop_count = 0;

        const int black_king = bit_scan_forward(board.bboard[(int)ColorPiece::BK]);
        const int white_king = bit_scan_forward(board.bboard[(int)ColorPiece::WK]);
        const int WRC = count_bits(board.bboard[(int)ColorPiece::WR]);
        const int BRC = count_bits(board.bboard[(int)ColorPiece::BR]);
        const int BQC = count_bits(board.bboard[(int)ColorPiece::BQ]);
        const int WQC = count_bits(board.bboard[(int)ColorPiece::WQ]);
        const int BNC = count_bits(board.bboard[(int)ColorPiece::BN]);
        const int WNC = count_bits(board.bboard[(int)ColorPiece::WN]);
        const int WBC = count_bits(board.bboard[(int)ColorPiece::WB]);
        const int BBC = count_bits(board.bboard[(int)ColorPiece::BB]);
        const int white_piece_material =  WQC * 900 + WRC * 500 + WBC * 300 + WNC * 300;
        const int black_piece_material =  BQC * 900 + BRC * 500 + BBC * 300 + BNC * 300;
        const int total_material = white_piece_material + black_piece_material;
        if (total_material <= 300 && (board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) == 0) return 0;
        bitboard bb;
        bb = board.bboard[(int)ColorPiece::WP];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            op_score += PAWN_SQUARE_TABLE[sq];
            eg_score += PAWN_SQUARE_TABLE_ENDGAME[sq];
            const auto doubled_pawns = count_bits(board.bboard[(int)ColorPiece::WP]&Board::GetFileMaskSquare(sq));
            if (doubled_pawns > 1) score-=DOUBLED_PAWN_PENALTY;
            if (!(board.bboard[(int)ColorPiece::BP] & Board::GetPassedMaskSquare<true>(sq))) score+=PASSED_PAWN_BONUS[sq/8];
            if (!(board.bboard[(int)ColorPiece::WP] & Board::GetIsolatedMaskSquare(sq))) score-=ISOLATED_PAWN_PENALTY;
        }
        //std::cout << score << ' ' << op_score << ' ' << eg_score << '\n';
        bb = board.bboard[(int)ColorPiece::BP];
        while (bb) {
            const auto sq = flip[bit_scan_forward(bb)];reset_lsb(bb);
            op_score -= PAWN_SQUARE_TABLE[sq];
            eg_score -= PAWN_SQUARE_TABLE_ENDGAME[sq];
            const auto doubled_pawns = count_bits(board.bboard[(int)ColorPiece::BP]&Board::GetFileMaskSquare(sq));
            if (doubled_pawns > 1) score+=DOUBLED_PAWN_PENALTY;
            if (!(board.bboard[(int)ColorPiece::WP] & Board::GetPassedMaskSquare<false>(flip[sq]))) score-=PASSED_PAWN_BONUS[sq/8];
            if (!(board.bboard[(int)ColorPiece::BP] & Board::GetIsolatedMaskSquare(sq))) score+=ISOLATED_PAWN_PENALTY;
        }
        //std::cout << score << ' ' << op_score << ' ' << eg_score << '\n';
        bb = board.bboard[(int)ColorPiece::WB];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score+=BISHOP_SQUARE_TABLE[sq];
            score += BISHOP_MOBILITY_BONUS * (count_bits(Board::BishopAttacks(sq, board.cur_all) & board.not_own) - 5);
            black_king_proximity_bonus += square_proximity[black_king][sq] * BISHOP_KING_PROXIMITY_BONUS;
            bishop_count++;
        }
        if (bishop_count==2) score += BISHOP_PAIR_BONUS;
        bishop_count=0;
        bb = board.bboard[(int)ColorPiece::BB];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score-=BISHOP_SQUARE_TABLE[flip[sq]];
            score -= BISHOP_MOBILITY_BONUS * (count_bits(Board::BishopAttacks(sq, board.cur_all) & board.not_own) - 5);
            white_king_proximity_bonus += square_proximity[white_king][sq] * BISHOP_KING_PROXIMITY_BONUS;
            bishop_count++;
        }
        if (bishop_count==2) score -= BISHOP_PAIR_BONUS;

        bb = board.bboard[(int)ColorPiece::WN];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score+=KNIGHT_SQUARE_TABLE[sq];
            black_king_proximity_bonus += square_proximity[black_king][sq] * KNIGHT_KING_PROXIMITY_BONUS;
        }
        bb = board.bboard[(int)ColorPiece::BN];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score-=KNIGHT_SQUARE_TABLE[flip[sq]];
            white_king_proximity_bonus += square_proximity[white_king][sq] * KNIGHT_KING_PROXIMITY_BONUS;
        }
        bb = board.bboard[(int)ColorPiece::WR];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score+=ROOK_SQUARE_TABLE[sq];
            score += ROOK_MOBILITY_BONUS * (count_bits(Board::RookAttacks(sq, board.cur_all) & board.not_own) - 5);
            black_king_proximity_bonus += square_proximity[black_king][sq] * ROOK_KING_PROXIMITY_BONUS;
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score+=OPEN_FILE_BONUS;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score+=SEMI_OPEN_FILE_BONUS;
        }
        bb = board.bboard[(int)ColorPiece::BR];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score-=ROOK_SQUARE_TABLE[flip[sq]];
            score -= ROOK_MOBILITY_BONUS * (count_bits(Board::RookAttacks(sq, board.cur_all) & board.not_own) - 5);
            white_king_proximity_bonus += square_proximity[white_king][sq] * ROOK_KING_PROXIMITY_BONUS;
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score-=OPEN_FILE_BONUS;
            else if (!(board.bboard[(int)ColorPiece::BP] & Board::GetFileMaskSquare(sq))) score-=SEMI_OPEN_FILE_BONUS;
        }
        bb = board.bboard[(int)ColorPiece::WQ];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score+=QUEEN_SQUARE_TABLE[sq];
            black_king_proximity_bonus += square_proximity[black_king][sq] * QUEEN_KING_PROXIMITY_BONUS;
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score+=OPEN_FILE_BONUS;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score+=SEMI_OPEN_FILE_BONUS;
        }
        bb = board.bboard[(int)ColorPiece::BQ];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            score-=QUEEN_SQUARE_TABLE[flip[sq]];
            white_king_proximity_bonus += square_proximity[white_king][sq] * QUEEN_KING_PROXIMITY_BONUS;
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score-=OPEN_FILE_BONUS;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score-=SEMI_OPEN_FILE_BONUS;
        }
        eg_score+=KING_SQUARE_TABLE_ENDGAME[white_king];
        op_score+=KING_SQUARE_TABLE[white_king];
        eg_score-=KING_SQUARE_TABLE_ENDGAME[flip[black_king]];
        op_score-=KING_SQUARE_TABLE[flip[black_king]];
        int white_king_safety = 0;
        int black_king_safety = 0;
        //add open filed around the king
        //add pawns in front of the king
        // add no pawns on the two squares in front of the king
        white_king_safety*=black_piece_material;
        black_king_proximity_bonus*=white_piece_material;
        black_king_safety*=white_piece_material;
        white_king_proximity_bonus*=black_piece_material;

        score += (black_king_proximity_bonus - white_king_proximity_bonus + black_king_safety - white_king_safety - eg_score * total_material + op_score * total_material)/MAX_MATERIAL;
	    score += eg_score;

        //std::cout << score << ' ' << board.GetFen() << '\n';
        if constexpr (IsWhite) return score;
        return -score;
    }
    #elif defined(VICE_EXPERIMENTAL)
    //VICE evaluation
    static constexpr int PieceVal[13] = { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000 };
    static constexpr int PawnIsolated = 10;
    static constexpr int PawnDoubled = 6;
    static constexpr int PawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };
    static constexpr int RookOpenFile = 10;
    static constexpr int RookSemiOpenFile = 5;
    static constexpr int QueenOpenFile = 5;
    static constexpr int QueenSemiOpenFile = 3;
    static constexpr int BishopPair = 30;
    static constexpr int BishopMobility = 2;
    static constexpr int KnightMobility = 2;
    static constexpr int RookMobility = 4;

    const int PawnTable[64] = {
        0,	0,	0,	0,	0,	0,	0,	0,
        10,	10,	0,	-10,-10,0,	10,	10,
        5,	0,	0,	5,	5,	0,	0,	5,
        0,	0,	10,	20,	20,	10,	0,	0,
        5,	5,	5,	10,	10,	5,	5,	5,
        10,	10,	10,	20,	20,	10,	10,	10,
        20,	20,	20,	30,	30,	20,	20,	20,
        0,	0,	0,	0,	0,	0,	0,	0
    };

    const int KnightTable[64] = {
        0,	-10,0,	0,	0,	0,	-10,0,
        0,	0,	0,	5,	5,	0,	0,	0,
        0,	0,	10,	10,	10,	10,	0,	0,
        0,	5,	10,	20,	20,	10,	5,	0,
        5,	10,	15,	20,	20,	15,	10,	5,
        5,	10,	10,	20,	20,	10,	10,	5,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	0,	0,	0,	0,	0,	0
    };

    const int BishopTable[64] = {
        0,	0,	-10,0,	0,	-10,0,	0,
        0,	0,	0,	10,	10,	0,	0,	0,
        0,	0,	10,	15,	15,	10,	0,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	0,	10,	15,	15,	10,	0,	0,
        0,	0,	0,	10,	10,	0,	0,	0,
        0,	0,	0,	0,	0,	0,	0,	0
    };

    const int RookTable[64] = {
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        25,	25,	25,	25,	25,	25,	25,	25,
        0,	0,	5,	10,	10,	5,	0,	0
    };

    const int KingE[64] = {
        -50,-10,0,	0,	0,	0,	-10,-50,
        -10,0,	10,	10,	10,	10,	0,	-10,
        0,	10,	15,	15,	15,	15,	10,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	10,	15,	15,	15,	15,	10,	0,
        -10,0,	10,	10,	10,	10,	0,	-10,
        -50,-10,0,	0,	0,	0,	-10,-50
    };

    const int KingO[64] = {
        0,	5,	5,	-10,-10,0,	10,	5,
        -30,-30,-30,-30,-30,-30,-30,-30,
        -50,-50,-50,-50,-50,-50,-50,-50,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70
    };
    static constexpr int flip[64] = {
        56,  57,  58,  59,  60,  61,  62,  63,
        48,  49,  50,  51,  52,  53,  54,  55,
        40,  41,  42,  43,  44,  45,  46,  47,
        32,  33,  34,  35,  36,  37,  38,  39,
        24,  25,  26,  27,  28,  29,  30,  31,
        16,  17,  18,  19,  20,  21,  22,  23,
        8,   9,  10,  11,  12,  13,  14,  15,
        0,   1,   2,   3,   4,   5,   6,   7
    };

    #define ENDGAME_MAT (PieceVal[(int)ColorPiece::WR] + 2 * PieceVal[(int)ColorPiece::WN] + 2 * PieceVal[(int)ColorPiece::WP])
    #define START_MAT (PieceVal[(int)ColorPiece::WR] * 2 + PieceVal[(int)ColorPiece::WN] * 2 + PieceVal[(int)ColorPiece::WB] * 2 + PieceVal[(int)ColorPiece::WQ] + PieceVal[(int)ColorPiece::WP] * 8)
    template<bool IsWhite>
    int Evaluate() {
        const auto whitebb = board.bboard[1] | board.bboard[2] | board.bboard[3] | board.bboard[4] | board.bboard[5] | board.bboard[6];
        const auto blackbb = board.bboard[7] | board.bboard[8] | board.bboard[9] | board.bboard[10] | board.bboard[11] | board.bboard[12];
        int score = 0;
        int wbishops = 0, bbishops = 0;
        int bmaterial = 0, wmaterial = 0;
        int same_color_bishops = 0;
        //todo: test passed pawns
        //pos->material[WHITE] - pos->material[BLACK];
        bitboard bb;
        bb = board.bboard[(int)ColorPiece::WP];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WP];
            score+=PawnTable[sq];
            const int doubled_pawns = count_bits(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq));
            score-=int(doubled_pawns>1) * PawnDoubled;
            if (!(board.bboard[(int)ColorPiece::BP] & Board::GetPassedMaskSquare<true>(sq))) score+=PawnPassed[sq/8];
            if (!(board.bboard[(int)ColorPiece::WP] & Board::GetIsolatedMaskSquare(sq))) score-=PawnIsolated;
        }
        bb = board.bboard[(int)ColorPiece::BP];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BP];
            score-=PawnTable[flip[sq]];
            const int doubled_pawns = count_bits(board.bboard[(int)ColorPiece::BP] & Board::GetFileMaskSquare(sq));
            score+=int(doubled_pawns>1) * PawnDoubled;
            if (!(board.bboard[(int)ColorPiece::WP] & Board::GetPassedMaskSquare<false>(sq))) score-=PawnPassed[flip[sq]/8];
            if (!(board.bboard[(int)ColorPiece::BP] & Board::GetIsolatedMaskSquare(sq))) score+=PawnIsolated;
        }
        bb = board.bboard[(int)ColorPiece::WN];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WN];
            score+=KnightTable[sq];
            score+=KnightMobility * (count_bits(Board::KnightAttacks(sq) & ~whitebb) - 3);
        }
        bb = board.bboard[(int)ColorPiece::BN];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BN];
            score-=KnightTable[flip[sq]];
            score-=KnightMobility * (count_bits(Board::KnightAttacks(sq) & ~blackbb) - 3);
        }
        bb = board.bboard[(int)ColorPiece::WB];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WB];
            score+=BishopTable[sq];
            score += BishopMobility * (count_bits(Board::BishopAttacks(sq, board.cur_all) & ~whitebb) - 5);
            wbishops++;
            same_color_bishops += int(sq%2)*2-1;
        }
        bb = board.bboard[(int)ColorPiece::BB];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BB];
            score-=BishopTable[flip[sq]];
            score -= BishopMobility * (count_bits(Board::BishopAttacks(sq, board.cur_all) & ~blackbb) - 5);
            same_color_bishops += int(sq%2)*2-1;
            bbishops++;
        }
        bb = board.bboard[(int)ColorPiece::WR];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WR];
            score+=RookTable[sq];
            score += RookMobility * (count_bits(Board::RookAttacks(sq, board.cur_all) & ~whitebb) - 5);
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score+=RookOpenFile;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score+=RookSemiOpenFile;
        }
        bb = board.bboard[(int)ColorPiece::BR];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BR];
            score-=RookTable[flip[sq]];
            score -= RookMobility * (count_bits(Board::RookAttacks(sq, board.cur_all) & ~blackbb) - 5);
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score-=RookOpenFile;
            else if (!(board.bboard[(int)ColorPiece::BP] & Board::GetFileMaskSquare(sq))) score-=RookSemiOpenFile;
        }
        bb = board.bboard[(int)ColorPiece::WQ];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WQ];
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score+=QueenOpenFile;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score+=QueenSemiOpenFile;
        }
        bb = board.bboard[(int)ColorPiece::BQ];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BQ];
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score-=QueenOpenFile;
            else if (!(board.bboard[(int)ColorPiece::BP] & Board::GetFileMaskSquare(sq))) score-=QueenSemiOpenFile;
        }
        const auto wking = bit_scan_forward(board.bboard[(int)ColorPiece::WK]);
        if (bmaterial <= ENDGAME_MAT) score += KingE[wking];
        else score += KingO[wking];
        const auto bking = flip[bit_scan_forward(board.bboard[(int)ColorPiece::BK])];
        if (wmaterial <= ENDGAME_MAT) score -= KingE[bking];
        else score -= KingO[bking];

        if(wbishops >= 2) score += BishopPair;
        if(bbishops >= 2) score -= BishopPair;

        score+=wmaterial-bmaterial;

        /*
        TODO: TEST
        if (score > 0 && board.bboard[(int)ColorPiece::WP] == 0) {
            if (wmaterial <= 400) score = 0;
        }
        else if (score < 0 && board.bboard[(int)ColorPiece::BP] == 0) {
            if (bmaterial <= 400) score = 0;
        }
        if (wbishops == 1 && bbishop == 1 && !same_color_bishops) score = score*4/5;
        */

        score = std::clamp(score, -EVAL_MATE+50, EVAL_MATE-50);
        if constexpr (IsWhite) return score;
        return -score;
    }
    #else
    //VICE evaluation
    static constexpr int PieceVal[13] = { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000 };
    static constexpr int PawnIsolated = 10;
    static constexpr int PawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };
    static constexpr int RookOpenFile = 10;
    static constexpr int RookSemiOpenFile = 5;
    static constexpr int QueenOpenFile = 5;
    static constexpr int QueenSemiOpenFile = 3;
    static constexpr int BishopPair = 30;

    const int PawnTable[64] = {
        0,	0,	0,	0,	0,	0,	0,	0,
        10,	10,	0,	-10,-10,0,	10,	10,
        5,	0,	0,	5,	5,	0,	0,	5,
        0,	0,	10,	20,	20,	10,	0,	0,
        5,	5,	5,	10,	10,	5,	5,	5,
        10,	10,	10,	20,	20,	10,	10,	10,
        20,	20,	20,	30,	30,	20,	20,	20,
        0,	0,	0,	0,	0,	0,	0,	0
    };

    const int KnightTable[64] = {
        0,	-10,0,	0,	0,	0,	-10,0,
        0,	0,	0,	5,	5,	0,	0,	0,
        0,	0,	10,	10,	10,	10,	0,	0,
        0,	5,	10,	20,	20,	10,	5,	0,
        5,	10,	15,	20,	20,	15,	10,	5,
        5,	10,	10,	20,	20,	10,	10,	5,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	0,	0,	0,	0,	0,	0
    };

    const int BishopTable[64] = {
        0,	0,	-10,0,	0,	-10,0,	0,
        0,	0,	0,	10,	10,	0,	0,	0,
        0,	0,	10,	15,	15,	10,	0,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	0,	10,	15,	15,	10,	0,	0,
        0,	0,	0,	10,	10,	0,	0,	0,
        0,	0,	0,	0,	0,	0,	0,	0
    };

    const int RookTable[64] = {
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        0,	0,	5,	10,	10,	5,	0,	0,
        25,	25,	25,	25,	25,	25,	25,	25,
        0,	0,	5,	10,	10,	5,	0,	0
    };

    const int KingE[64] = {
        -50,-10,0,	0,	0,	0,	-10,-50,
        -10,0,	10,	10,	10,	10,	0,	-10,
        0,	10,	15,	15,	15,	15,	10,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	10,	15,	20,	20,	15,	10,	0,
        0,	10,	15,	15,	15,	15,	10,	0,
        -10,0,	10,	10,	10,	10,	0,	-10,
        -50,-10,0,	0,	0,	0,	-10,-50
    };

    const int KingO[64] = {
        0,	5,	5,	-10,-10,0,	10,	5,
        -30,-30,-30,-30,-30,-30,-30,-30,
        -50,-50,-50,-50,-50,-50,-50,-50,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70,
        -70,-70,-70,-70,-70,-70,-70,-70
    };
    static constexpr int flip[64] = {
        56,  57,  58,  59,  60,  61,  62,  63,
        48,  49,  50,  51,  52,  53,  54,  55,
        40,  41,  42,  43,  44,  45,  46,  47,
        32,  33,  34,  35,  36,  37,  38,  39,
        24,  25,  26,  27,  28,  29,  30,  31,
        16,  17,  18,  19,  20,  21,  22,  23,
        8,   9,  10,  11,  12,  13,  14,  15,
        0,   1,   2,   3,   4,   5,   6,   7
    };

    #define ENDGAME_MAT (PieceVal[(int)ColorPiece::WR] + 2 * PieceVal[(int)ColorPiece::WN] + 2 * PieceVal[(int)ColorPiece::WP])
    template<bool IsWhite>
    int Evaluate() {
        int score = 0;
        int wbishops = 0, bbishops = 0;
        int bmaterial = 0, wmaterial = 0;
        //todo: test passed pawns
        //pos->material[WHITE] - pos->material[BLACK];
        bitboard bb;
        bb = board.bboard[(int)ColorPiece::WP];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WP];
            score+=PawnTable[sq];
            if (!(board.bboard[(int)ColorPiece::BP] & Board::GetPassedMaskSquare<true>(sq))) score+=PawnPassed[sq/8];
            if (!(board.bboard[(int)ColorPiece::WP] & Board::GetIsolatedMaskSquare(sq))) score-=PawnIsolated;
        }
        bb = board.bboard[(int)ColorPiece::BP];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BP];
            score-=PawnTable[flip[sq]];
            if (!(board.bboard[(int)ColorPiece::WP] & Board::GetPassedMaskSquare<false>(sq))) score-=PawnPassed[flip[sq]/8];
            if (!(board.bboard[(int)ColorPiece::BP] & Board::GetIsolatedMaskSquare(sq))) score+=PawnIsolated;
        }
        bb = board.bboard[(int)ColorPiece::WN];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WN];
            score+=KnightTable[sq];
        }
        bb = board.bboard[(int)ColorPiece::BN];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BN];
            score-=KnightTable[flip[sq]];
        }
        bb = board.bboard[(int)ColorPiece::WB];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WB];
            score+=BishopTable[sq];
            wbishops++;
        }
        bb = board.bboard[(int)ColorPiece::BB];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BB];
            score-=BishopTable[flip[sq]];
            bbishops++;
        }
        bb = board.bboard[(int)ColorPiece::WR];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WR];
            score+=RookTable[sq];
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score+=RookOpenFile;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score+=RookSemiOpenFile;
        }
        bb = board.bboard[(int)ColorPiece::BR];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BR];
            score-=RookTable[flip[sq]];
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score-=RookOpenFile;
            else if (!(board.bboard[(int)ColorPiece::BP] & Board::GetFileMaskSquare(sq))) score-=RookSemiOpenFile;
        }
        bb = board.bboard[(int)ColorPiece::WQ];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            wmaterial+=PieceVal[(int)ColorPiece::WQ];
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score+=QueenOpenFile;
            else if (!(board.bboard[(int)ColorPiece::WP] & Board::GetFileMaskSquare(sq))) score+=QueenSemiOpenFile;
        }
        bb = board.bboard[(int)ColorPiece::BQ];
        while (bb) {
            const auto sq = bit_scan_forward(bb);reset_lsb(bb);
            bmaterial+=PieceVal[(int)ColorPiece::BQ];
            if (!((board.bboard[(int)ColorPiece::WP] | board.bboard[(int)ColorPiece::BP]) & Board::GetFileMaskSquare(sq))) score-=QueenOpenFile;
            else if (!(board.bboard[(int)ColorPiece::BP] & Board::GetFileMaskSquare(sq))) score-=QueenSemiOpenFile;
        }
        const auto wking = bit_scan_forward(board.bboard[(int)ColorPiece::WK]);
        if (bmaterial <= ENDGAME_MAT) score += KingE[wking];
        else score += KingO[wking];
        const auto bking = flip[bit_scan_forward(board.bboard[(int)ColorPiece::BK])];
        if (wmaterial <= ENDGAME_MAT) score -= KingE[bking];
        else score -= KingO[bking];

        if(wbishops >= 2) score += BishopPair;
        if(bbishops >= 2) score -= BishopPair;

        score+=wmaterial-bmaterial;

        if constexpr (IsWhite) return score;
        return -score;
    }



    #endif
    public:
        static constexpr int promotion_score[13] = {0, 0, -20000, -40000, -30000, 10000, 0, 0, -20000, -40000, -30000, 10000, 0};
        static constexpr int mvv_lva[12][12] = {
            105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

            105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
        };
        Move killers[2][MAX_PLY] = {};
        Move history[12][64] = {};
        int pv_table[MAX_PLY][MAX_PLY];
        int pv_length[MAX_PLY];
        int lmred[2][33][64];
        int square_proximity[64][64];
    };
    
    namespace UCI{
        Move parse_movestr(const std::string& str, Board& board) {
            MoveList list;
            if (board.IsWhiteToMove()) 
                board.GenMovesUnchecked<true>(list);
            else 
                board.GenMovesUnchecked<false>(list);
            std::string temp;
            if (str.size() == 5) temp = str.substr(0, 5);
            else temp = str.substr(0, 4);
            for (int i = 0; i < list.count; ++i) {
                if (temp == move_to_string(list.move_storage[i])) return list.move_storage[i];
            }
            std::cout << "Move error! " << str << std::endl;
            return NullMove;
            //Move move; 
            //char sq1 = (str[0]-'a') * 8 + (str[1]-'1');
            //char sq2 = (str[2]-'a') * 8 + (str[3]-'1');
            
        }
        void uci_loop(SearchEngine& engine) {
            // print engine info
            engine.board.Reset();
            std::atomic_bool quiting = false;
            std::thread search_thread, timethread;
            std::cout << "Simple chess engine by motya\n";
            std::cout << "communicate using UCI\n";
            //std::cout << "For more info write   help  \n";
            std::string cmd;
            bool timeset = false, time_up = false, last_was_startpos = false;
            UCI::stopped = true;
            while (true) {
                std::cout.flush();
                std::cin >> cmd;
                if (!stopped) {
                    if (cmd == "stop") {
                        stopped = true;
                    }
                    else if (cmd == "quit") {
                        quiting = true;
                        stopped = true;
                        search_thread.join();
                        return;
                    }
                    else {
                        std::cout << "Unknown command\n";
                    }
                    continue;
                }
                if (cmd == "") continue;
                if (cmd == "isready") {
                    std::cout << "readyok\n";
                }
                else if (cmd == "debug") {
                    std::cin >> cmd;
                    if (cmd == "on") {
                        UCI::uci_debug = true;
                    }
                    else if (cmd == "off") {
                        UCI::uci_debug = false;
                    }
                    else {
                        std::cout << "Unknown command\n";
                    }
                }
                else if (cmd == "ucinewgame") {
                    engine.board.SetFen(startpos);
                    engine.look_at_book = true;
                }
                else if (cmd == "position") {
                    //parse position
                    std::getline(std::cin, cmd);
                    std::stringstream ss(cmd);
                    ss >> cmd;
                    if (cmd == "startpos") {
                        engine.board.SetFen(startpos);
                        engine.look_at_book = true;
                    }
                    else if (cmd == "fen") {
                        ss >> cmd;
                        std::string fen = ss.str().substr(5);
                        if (!engine.board.SetFen(fen)) {
                            std::cout << "Invalid fen\n";
                        }
                        ss >> cmd;ss >> cmd; ss >> cmd; ss >> cmd;ss >> cmd;
                    }
                    else continue;
                    if (!(ss>>cmd)) {
                        if (last_was_startpos) {
                            engine.board.SetFen(startpos);
                            engine.look_at_book = true;
                            last_was_startpos = false;
                        }
                    }
                    else if (cmd == "moves") {
                        repetition_table[++repetition_index] = engine.board.current_key;
                        while (ss >> cmd) {
                            Move move = parse_movestr(cmd, engine.board);
                            if (!move) exit(1);
                            if (engine.board.move_count & 1) engine.board.MakeMove<true>(move);
                            else engine.board.MakeMove<false>(move);
                            repetition_table[++repetition_index] = engine.board.current_key;
                        }
                    }
                    //engine.board.ShowBoard();
                    //std::cout << engine.board.GetFen() << '\n';
                    std::cout << "Hash key: " << engine.board.current_key << '\n';
                    if (engine.board.IsWhiteToMove()) std::cout << "White to move\n";
                    else  std::cout << "Black to move\n\n";
                }
                else if (cmd == "go") {
                    //parse go
                    std::getline(std::cin, cmd);
                    std::stringstream ss(cmd);
                    int movetime = -1, time = -1, inc = 0, movestogo = 40, depth = -1;
                    UCI::starttime = 0;UCI::stoptime=0;
                    timeset = false;
                    while (ss >> cmd) {
                        if (cmd == "infinite") {}
                        else if (cmd == "perft") {
                            ss >> depth;
                            std::cout << "perft divide\n";
                            if (engine.board.IsWhiteToMove()) engine.perft_divide<true>(depth);
                            else engine.perft_divide<false>(depth);
                            std::cout.flush();
                            break;
                        }
                        else if (cmd == "wtime") {
                            ss >> cmd;
                            if (engine.board.IsWhiteToMove()) time = std::stoi(cmd);
                        }
                        else if (cmd == "btime") {
                            ss >> cmd;
                            if (!engine.board.IsWhiteToMove()) time = std::stoi(cmd);
                        }
                        else if (cmd == "winc") {
                            ss >> cmd;
                            if (engine.board.IsWhiteToMove()) inc = std::stoi(cmd);
                        }
                        else if (cmd == "binc") {
                            ss >> cmd;
                            if (!engine.board.IsWhiteToMove()) inc = std::stoi(cmd);
                        }
                        else if (cmd == "movestogo") {
                            ss >> cmd;
                            movestogo = std::stoi(cmd);
                        }
                        else if (cmd == "movetime") {
                            ss >> cmd;
                            movetime = std::stoi(cmd);
                        }
                        else if (cmd == "depth") {
                            ss >> cmd;
                            depth = std::stoi(cmd);
                        }
                        else if (cmd == "nodes") {
                            ss >> cmd;
                            std::cout << "Not supported yet(:\n";
                        }
                        else if (cmd == "movetime") {
                            ss >> cmd;
                            movetime = std::stoi(cmd);
                        }
                        else if (cmd == "searchmoves") {
                            std::cout << "Not supported yet(:\n";
                        }
                        else if (cmd == "ponder") {
                            std::cout << "Not supported yet(:\n";
                        }
                        else {
                            std::cout << "Unknown command(:\n";
                        }
                    }
                    if (cmd == "perft") continue;
                    if(movetime != -1)
                    {
                        time = movetime;
                        movestogo = 1;
                    }
                    starttime = get_time_ms();
                    //std::cout << time << ' ' << engine.board.IsWhiteToMove() << '\n';
                    if(time != -1)
                    {
                        timeset = 1;
                        time /= movestogo;
                        time -= 25;
                        //almost no time left
                        if (time < 0) {
                            time_up = true;
                            time = 0;
                            //compensation
                            inc -= 25;
                            
                            if (inc < 0) inc = 1;
                        }
                        stoptime = starttime + time + inc;  
                    }
                    if (depth == -1) depth = MAX_PLY-1;
                    std::cout << "time: "<< time <<"  start: " << starttime << "  stop: "<< stoptime << "  depth: "<< depth << "  timeset:"<< timeset <<"\n";
                    //-----thread manipulation-----
                    UCI::stopped = false;
                    if (search_thread.joinable()) {search_thread.join();}
                    if (timeset) {
                        if (engine.board.IsWhiteToMove()) search_thread = std::thread([&](){engine.go_position<true, true>(depth, stoptime, time_up);});
                        else search_thread = std::thread([&](){engine.go_position<false, true>(depth, stoptime, time_up);});
                    }else {
                        if (engine.board.IsWhiteToMove()) search_thread = std::thread([&](){engine.go_position<true, false>(depth, stoptime, time_up);});
                        else search_thread = std::thread([&](){engine.go_position<false, false>(depth, stoptime, time_up);});
                    }
                    //std::cout << "time: %d  start: %u  stop: %u  depth: %d  timeset:%d\n";
                }
                else if (cmd == "quit") {        
                    quiting = true;
                    stopped = true;
                    if (search_thread.joinable()) search_thread.join();
                    return;
                }
                else if (cmd == "uci" || cmd == "UCI") {
                    std::cout << "id name SCE++\n";
                    std::cout << "id author motya\n";
                    std::cout << "option name Hash type spin default 1000000 min 1 max " << max_hash_size << "\n";
                    //std::cout << "option name Threads type spin default 1 min 1 max " << std::thread::hardware_concurrency() << "\n";
                    //std::cout << "option name Ponder type check default false\n";
                    std::cout << "option name OwnBook type check default false\n";
                    std::cout << "uciok\n";
                }
                else if (cmd == "setoption") {
                    std::cin >> cmd;
                    if (cmd != "name") {
                        std::cout << "Unknown command(:\n";
                        continue;
                    }
                    std::cin >> cmd;
                    if (cmd == "Hash") {
                        std::cin >> cmd;
                        if (cmd != "value") {
                            std::cout << "Unknown command(:\n";
                            continue;
                        }
                        std::cin >> cmd;
                        clear_hash_table();
                        int new_hash_size = std::stoi(cmd);
                        if (hash_size > max_hash_size) hash_size = max_hash_size;
                        if (hash_size < 1) hash_size = 1;
                        #ifndef TEST_RELEASE
                        hash_size=new_hash_size;
                        //hash_table.resize(hash_size);
                        #endif
                    }
                    else if (cmd == "OwnBook") {
                        std::cin >> cmd;
                        if (cmd!="value") {
                            std::cout << "Unknown command(:\n";
                            continue;
                        }
                        std::cin >> cmd;
                        if (cmd == "true") engine.use_book = true;
                        else engine.use_book = false;
                    }
                    else std::cout << "Unknown option\n";
                }
                else std::cout << "Unknown command\n";
            }
        }
    }
}

int main() {
    using namespace ChessEngine;
    std::ios_base::sync_with_stdio(false);
    //UCI::uci_debug = true;
    Board::InitAll();
    auto engine = SearchEngine();
    engine.Init();
    UCI::uci_loop(engine);
    return 0;
}