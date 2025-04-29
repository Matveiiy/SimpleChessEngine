////
//// Created by motya on 29.04.2025.
////
/*
template<bool IsWhite>
uint64_t perft(int depth) {
    if (depth == 0) return 1;
    ChessEngine::MoveList move_list;
    board.GenMovesUnchecked<IsWhite>(move_list);
    uint64_t nodes = 0;
    for (int i = 0; i < move_list.count; ++i) {
        board.MakeMove<IsWhite>(move_list.move_storage[i]);
        if (board.IsKingInCheck<IsWhite>()) {board.UndoMove<IsWhite>(move_list.move_storage[i]);continue;}
        nodes+=perft<!IsWhite>(depth-1);
        board.UndoMove<IsWhite>(move_list.move_storage[i]);
    }
    return nodes;
}
template<bool IsWhite>
uint64_t divide_debug(int depth, std::string print_offset="", std::string right_fen = "") {
    if (depth == 0) return 1;
    ChessEngine::MoveList move_list;
    board.GenMovesUnchecked<IsWhite>(move_list);
    uint64_t nodes = 0;
    for (int i = 0; i < move_list.count; ++i) {
        std::cout << print_offset << "Move: ";ChessEngine::print_move(move_list.move_storage[i]);std::cout << '\n';
        std::cout << print_offset << "Current state: " << board.GetFen() << '\n';
        std::cout << ChessEngine::square_to_string(board.en_passant) << '\n';
        board.MakeMove<IsWhite>(move_list.move_storage[i]);
        std::cout << ChessEngine::square_to_string(board.en_passant) << '\n';
        std::cout << print_offset << "After move: " << board.GetFen() << '\n';
        //print_bitboard(board.bboard[(int)ChessEngine::ColorPiece::WK]);
        if (board.IsKingInCheck<IsWhite>()) {
            std::cout << print_offset << "Discarded\n";
            board.UndoMove<IsWhite>(move_list.move_storage[i]);
            continue;
        }
        auto f = divide_debug<!IsWhite>(depth-1, print_offset+"    ", board.GetFen());
        //std::cout << "1h\n";
        //ChessEngine::print_movestd(move_list.move_storage[i]);
        //std::cout << ": " << x << '\n';
        nodes+=f;
        board.UndoMove<IsWhite>(move_list.move_storage[i]);
    }
    std::cout << print_offset << "Total number of nodes: " << nodes << '\n';
    return nodes;
}
template<bool IsWhite>
void divide(int depth) {
    if (depth == 0) return;
    ChessEngine::MoveList move_list;
    board.GenMovesUnchecked<IsWhite>(move_list);
    uint64_t nodes = 0;
    for (int i = 0; i < move_list.count; ++i) {
        board.MakeMove<IsWhite>(move_list.move_storage[i]);
        if (board.IsKingInCheck<IsWhite>()) {
            board.UndoMove<IsWhite>(move_list.move_storage[i]);
            continue;
        }
        auto x = perft<!IsWhite>(depth-1);
        ChessEngine::print_move_std(move_list.move_storage[i]);
        std::cout << ": " << x << '\n';
        nodes+=x;
        board.UndoMove<IsWhite>(move_list.move_storage[i]);
    }
    std::cout << "Total number of nodes: " << nodes << '\n';
    return;
}*/
//struct PerftTest{
//public:
//    std::string fen;
//    std::vector<long long> res;
//    int max_depth = 0;
//    PerftTest(std::string FEN = "", std::initializer_list<long long> list = {}, int depth_max = 0):res(list) {fen = FEN;max_depth = depth_max;if (max_depth == 0) max_depth = res.size(); }
//};
//struct PerftTestError{
//public:
//    int test_id, depth;
//    long long incorrect, correct;
//    PerftTestError(int Testid, int Depth, long long Incorrect, long long Correct):test_id(Testid), depth(Depth), incorrect(Incorrect), correct(Correct) {}
//};
//std::vector<PerftTest> _tests = {
//        PerftTest("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20,400,8902,197281,4865609,119060324,3195901860}, 5),
//        PerftTest("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48,2039,97862,4085603,193690690,8031647685}, 5),
//        PerftTest("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14,191,2812,43238,674624,11030083,178633661,3009794393}, 6),
//        PerftTest("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6,264,9467,422333,15833292,706045033}, 5),
//        PerftTest("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ", {44,1486,62379,2103487,89941194}),
//        PerftTest("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46,2079,89890,3894594,164075551,6923051137,287188994746,11923589843526,490154852788714}, 5),
//        //r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10
//};
//void test(PerftTest& test, std::vector<PerftTestError>& log, int test_id, std::ofstream& fout, ChessEngine::SearchEngine& engine) {
//    if (!engine.board.SetFen(test.fen)) {log.push_back({test_id, 0, 0, 0}); return;}
//    for (int d = 1; d <= test.max_depth; ++d) {
//        std::cout << "    depth " << d << "...\n";
//        bitboard x;
//        if (engine.board.move_count & 1) x = engine.perft<true>(d);
//        else x = engine.perft<false>(d);
//        if (x != (uint64_t)test.res[d-1]) {
//            log.push_back(PerftTestError(test_id, d, x, test.res[d-1]));
//            fout << "Test " << test_id << " depth "<< d << " => incorrect result: (incorrect) " << x << " : " << test.res[d-1] << " (correct)\n";
//            return;
//        }
//    }
//}
//std::vector<PerftTest> tests_from_file;
//void read_tests(const char* filename, int from, int to) {
//    std::ifstream file(filename);
//    if (file.fail()) {std::cout << "Test file not found\n";return;}
//    std::string s, s2;
//    int id = 0;
//    while (!file.eof()) {
//        ++id;
//        std::getline(file, s);
//        if (id < from) {continue;}
//        if (id > to) {break;}
//        PerftTest testc;
//        std::vector<std::string> arr;
//        split(s, ',', arr);
//        testc.fen = arr[0] + " 0 1";
//        testc.max_depth = 5;
//        testc.res.resize(6);
//        for (int i = 0; i < 6; ++i) testc.res[i] = stoll(arr[i+1]);
//        tests_from_file.push_back(testc);
//    }
//    std::cout << "Tests are parsed succesfully\n";
//}
//void test_all(std::vector<PerftTest>& tests, ChessEngine::SearchEngine& engine) {
//    std::cout << "Testing...\n";
//    std::ofstream test_results("test_results.log");
//    std::vector<PerftTestError> log;
//    for (int i = 1; i <= (int)tests.size(); ++i) {
//        std::cout << "Test " << i << '\n';
//        test(tests[i-1], log, i, test_results, engine);
//        if (i % 10) test_results.flush();
//    }
//    std::sort(log.begin(), log.end(), [](const PerftTestError& left, const PerftTestError& right){
//        if (left.depth != right.depth) return left.depth < right.depth;
//        return left.incorrect < right.incorrect;
//        //if (left.incorrect!=right.incorrect)
//    });
//    for (auto& x : log) {
//        if (x.depth==0) std::cout << "Invalid test " << x.test_id;
//        std::cout << "Test " << x.test_id << " depth " << x.depth << " => incorrect result: (incorrect) " << x.incorrect << " : " <<  x.correct << " (correct)\n";
//    }
//    if (log.size()) std::cout << "Failed\n";
//    else std::cout << "Passed\n";
//}
//
//using namespace ChessEngine;
//template<bool IsWhite, bool captures_only>
//inline void debug_search(SearchEngine& engine) {
//    auto eval = engine.Evaluate<IsWhite>();
//    MoveList moves;
//    std::cout << eval << '\n' << '\n';
//    engine.board.GenMovesUnchecked<IsWhite>(moves);
//    for (int i = 0; i < moves.count; ++i) {
//        auto move = moves.move_storage[i];
//        engine.board.MakeMove<IsWhite>(move);
//        if constexpr (captures_only) {
//            if (GetMoveCapture(move) == 0) {
//                engine.board.UndoMove<IsWhite>(move);
//                continue;
//            }
//        }
//        if (engine.board.IsKingInCheck<IsWhite>()) {
//            engine.board.UndoMove<IsWhite>(move);
//            continue;
//        }
//        std::cout << move_to_string(move) << '\n';
//        ++engine.ply;
//        int score = -engine.quiet_search<!IsWhite, false>(-engine.EVAL_INFINITY, -eval);
//        std::cout << score << '\n';
//        if (score>eval){
//            eval = score;
//            std::cout << "Updated\n";
//        }
//        engine.board.UndoMove<IsWhite>(move);
//        --engine.ply;
//    }
//    std::cout << '\n' << eval << '\n';
//}
//int main() {
//#define SetBoardFen(ss) if (!engine.board.SetFen((ss))) {std::cout << "Invalid fen\n";return 0; }
//    //todo:
//    // follow pv after null move pruning?
//    // add queen table
//    // add bishop, queen mobility
//    // check passed pawn eval black
//    // add bishop pair and queen open file
//    // maybe genereate only legal moves
//    // remove assert in evaluation(if it works)
//    //  change endgame phase score
//    //  quiescene: custom score move(for captures only)
//    //  bug when discarded by aspiration window
//    //  * SEE
//    //  * probcut
//    //  * rank cut
//    //  evaluation
//    //  hashing: transposition/zobrist table
//    //  move bishop&rook attacks to constexpr or consteval or constinit
//    //  very long mate: R4r1k/6pp/2pq4/2n2b2/2Q1pP1b/1r2P2B/NP5P/2B2KNR b - - 1 24
//    using namespace ChessEngine;
//    Board::InitAll();
//    SearchEngine engine;
//    engine.Init();
//    if (0) {
//        test_all(_tests, engine);
//        read_tests("tests/perft_tests.txt", 1, 50);
//        test_all(tests_from_file, engine);
//        //std::cout << board.GetFen() << '\n';
//        //std::cout << board.GetFen();
//        return 0;
//    }
//    if (0) {
//        for (int i = 0; i < 32; ++i) {
//            print_bitboard(Board::GetPassedMaskSquare<false>(i));
//            print_bitboard(Board::GetPassedMaskSquare<true>(i));
//        }
//        print_bitboard(Board::GetPassedMaskSquare<true>(62));
//        print_bitboard(Board::GetPassedMaskSquare<false>(2));
//        return 0;
//    }
//    if (1) {
//        //for (int i = 0; i < 64; ++i) {print_bitboard(Board::GetFileMaskSquare(i));}
//        //startpos
//        UCI::uci_debug = true;
//        //r5k1/1ppn1ppp/p2p1b2/5N2/2P1N3/5P2/P2r2PP/1R3R1K w - - 0 23
//        engine.board.SetFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
//        //engine.go_position<true, false>(1);
//        //return 0;
//        MoveList moves;
//        engine.board.GenMovesUnchecked<true>(moves);
//        int temp_pos=0;
//        temp_pos += engine.enable_pvs(moves);
//        temp_pos += engine.find_best(moves, NullMove, temp_pos);
//        engine.sort_moves(moves, temp_pos);
//        for (int i = 0; i < moves.count; ++i) {
//            std::cout << move_to_string(moves.move_storage[i]) << ' ' << engine.score_move(moves.move_storage[i]) << '\n';
//        }
//        std::cout << "------------------------------------------\n";
//        moves.count = 0;
//        engine.board.GenMovesUnchecked<true>(moves);
//        std::vector< std::pair<int, Move> > scores(moves.count);
//        for (int i = 0; i < moves.count; ++i) {
//            if (engine.follow_pv) if (moves.move_storage[i] == engine.pv_table[0][engine.ply]) scores[i].first = 2000000;
//                else if (moves.move_storage[i] == NullMove) scores[i].first = 1000000;
//                else scores[i].first = engine.score_move(moves.move_storage[i]);
//            scores[i].second = moves.move_storage[i];
//        }
//        //engine.QuickSort(moves.count-1, 0, scores, moves);
//        for (int i = 0; i < moves.count; ++i) {
//            std::cout << move_to_string(scores[i].second) << ' ' << scores[i].first << '\n';
//        }
//        //if (engine.board.IsWhiteToMove()) std::cout << engine.Evaluate<true>() << '\n';
//        //else std::cout << engine.Evaluate<false>() << '\n';
//        //debug_search<false, true>(engine);
//        return 0;
//    }
//    if (0) {
//        engine.board.SetFen("r5k1/1ppn1ppp/p2p1b2/5N2/2P1N3/5P2/P2Br1PP/1R3R1K b - - 3 22");
//        engine.go_position<false, false>(16, 0);
//        //board.SetFen("5Q2/1R6/k7/8/8/4PB2/5K1P/8 w - - 0 1");
//        //std::cout << "Perft: " << perft<true>(2) << '\n';
//
//        //r3k2r/pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1
//        //auto eval = engine.go_position(7, 0, 0);
//        //printf("%llx\n", engine.board.GenHashKey<true>());
//        return 0;
//    }
//    if (0) {
//        //c7c5
//        //e7e5
//        SetBoardFen("rnbqkbnr/1p1ppppp/2p5/p7/6P1/2P5/PP1PPP1P/RNBQKBNR w QKkq - 0 1");
//        //std::cout << board.castle << '\n';
//        MoveList moves;
//        engine.board.GenMovesUnchecked<true>(moves);
//        printf("%llx  %llx\n", engine.board.GenHashKey<true>(), engine.board.current_key);
//        engine.board.MakeMove<true>(moves.move_storage[0]);
//        printf("%llx  %llx\n", engine.board.GenHashKey<false>(), engine.board.current_key);
//        return 0;
//        //print_move(moves.move_storage[4]);std::cout << '\n';
//        //printf("%llx  %llx\n", board.GenHashKey<true>(), board.current_key);
//        //board.MakeMove<true>(moves.move_storage[4]);
//        //printf("%llx  %llx\n", board.GenHashKey<true>(), board.current_key);
//        //std::cout << board.GenHashKey<true>() << ' ' << board.current_key << '\n';
//        //std::cout << perft<true>(1) << '\n';
//        //divide_debug<false>(2);
//        return 0;
//        //MoveList list;
//        //board.GenMovesUnchecked<true>(list);
//        //for (int i = 0; i < list.count; ++i) {
//        //    auto x = list.move_storage[i];
//        //    std::cout << ChessEngine::move_to_string(x) << '\n';
//        //}
//    }
//
//    /*
//    engine.board.SetFen("1k6/8/8/3pP3/8/8/3K4/8 w - d6 0 2");
//    MoveList moves;
//    engine.board.GenMovesUnchecked<true>(moves);
//    for (int i = 0; i < moves.count; ++i) {
//        std::cout << "Move: ";
//        print_move(moves.move_storage[i]);
//        std::cout << '\n';
//    }
//    */
//    //SetBoardFen("2bqkbnr/rpp1p1pp/n2p4/5p2/p1PP2P1/PP5P/4PP2/RNBQKBNR w KQk - 0 1");
//    //read_tests("tests/perft_tests.txt", 0, 300);
//    //test_all(tests_from_file);
//    //test_all(_tests);
//
//    //read_tests("perft_tests.txt", 28, 28);
//    //std::cout << tests_from_file[0].fen << '\n';
//    //divide<true>(3);
//    //divide_debug<false>(2, "", board.GetFen());
//    //test_all(tests_from_file);
//    //test_all(_tests);
//    return 0;
//}