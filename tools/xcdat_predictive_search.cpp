#include <xcdat.hpp>

/* This is a fork of xcdat_predictive_search that modifies the record 
   delimiter so it functions as more than simply an evaluation tool. */

#include "cmd_line_parser/parser.hpp"
#include "mm_file/mm_file.hpp"
#include "tinyformat/tinyformat.h"

cmd_line_parser::parser make_parser(int argc, char** argv) {
    cmd_line_parser::parser p(argc, argv);
    p.add("input_dic", "Input filepath of trie dictionary");
    p.add("max_num_results", "The max number of results (default=10)", "-n", false);
    return p;
}

template <class Trie>
int predictive_search(const cmd_line_parser::parser& p) {
    const auto input_dic = p.get<std::string>("input_dic");
    const auto max_num_results = p.get<std::uint64_t>("max_num_results", 10);

    const mm::file_source<char> fin(input_dic.c_str(), mm::advice::sequential);
    const auto trie = xcdat::mmap<Trie>(fin.data());

    struct result_type {
        std::uint64_t id;
        std::string str;
    };
    std::vector<result_type> results;
    results.reserve(1ULL << 10);

    for (std::string key; std::getline(std::cin, key);) {
        results.clear();
        trie.predictive_search(key, [&](std::uint64_t id, std::string_view str) {
            results.push_back({id, std::string(str)});
        });

        tfm::printf("%d found\t", results.size());
        for (std::uint64_t i = 0; i < std::min<std::uint64_t>(results.size(), max_num_results); i++) {
            const auto& r = results[i];
//            tfm::printf("%d\t%s", r.id, r.str);
	    if (i != 0) { tfm::printf("\t"); }
            tfm::printf("%s", r.str);
        }
	std::cout << std::endl << std::flush;
    }

    return 0;
}

int main(int argc, char** argv) {
#ifndef NDEBUG
    tfm::warnfln("The code is running in debug mode.");
#endif
    std::ios::sync_with_stdio(false);

    auto p = make_parser(argc, argv);
    if (!p.parse()) {
        return 1;
    }

    const auto input_dic = p.get<std::string>("input_dic");
    const auto type_id = xcdat::get_type_id(input_dic);

    switch (type_id) {
        case 7:
            return predictive_search<xcdat::trie_7_type>(p);
        case 8:
            return predictive_search<xcdat::trie_8_type>(p);
        case 15:
            return predictive_search<xcdat::trie_15_type>(p);
        case 16:
            return predictive_search<xcdat::trie_16_type>(p);
        default:
            break;
    }

    p.help();
    return 1;
}
