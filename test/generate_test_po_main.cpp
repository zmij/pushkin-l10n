/*
 * generate_test_po_main.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "test_data_loader.hpp"
#include <pushkin/l10n/po_generator.hpp>

int
main(int argc, char* argv[])
try {
    namespace po = ::boost::program_options;
    ::std::string out_file_name{ "messages.pot" };
    po::options_description opts_desc{"Main options"};
    bool verbose{false};
    opts_desc.add_options()
        ("help,h", "show options description")
        ("output-file,o",
            po::value<::std::string>(&out_file_name)
                ->default_value("messages.pot"),
            "Name of output pot file")
        ("verbose,v",
            po::bool_switch(&verbose),
            "Be verbose")
    ;
    psst::l10n::po_generator gen;
    po::options_description cmd_line_desc;
    cmd_line_desc.add(opts_desc).add(gen.options_description());
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmd_line_desc), vm);

    if (vm.count("help")) {
        std::cout << cmd_line_desc << "\n";
        return 0;
    }

    po::notify(vm);

    psst::l10n::test::test_data_loader ldr;
    if (verbose)
        ::std::cerr << "There are " << ldr.messages().size()
                << " entries in the test file\n";

    for (auto const& msg : ldr.messages()) {
        gen.add_message(msg);
    }
    if (verbose)
        ::std::cerr << "Output file: " << out_file_name << "\n";

    ::std::ofstream ofs(out_file_name);
    if (!ofs)
        throw ::std::runtime_error{ "Failed to open output file '" + out_file_name + "'"};
    ofs << gen << "\n";

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Standard exception " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
