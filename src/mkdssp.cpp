// Copyright Maarten L. Hekkelman, Radboud University 2008-2011.
//   Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)
//
// A DSSP reimplementation

#include "dssp.h"
#include "iocif.h"
#include "mas.h"
#include "structure.h"
#include "version.h"

#include <boost/program_options.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#if defined USE_COMPRESSION
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#endif
#include <boost/algorithm/string.hpp>

#if defined(_MSC_VER)
#include <conio.h>
#include <ctype.h>
#endif
#include <fstream>


namespace po = boost::program_options;
namespace io = boost::iostreams;
namespace ba = boost::algorithm;

//int VERBOSE = 0;

int main(int argc, char* argv[])
{
  try
  {
    po::options_description desc("mkdssp " XSSP_VERSION " options");
    desc.add_options()
      ("help,h", "Display help message")
      ("input,i", po::value<std::string>(), "Input file")
      ("output,o",
       po::value<std::string>(),
       "Output file, use 'stdout' to output to screen")
      ("verbose,v", "Verbose output")
      ("version", "Print version")
      ("debug,d",
       po::value<int>(),
       "Debug level (for even more verbose output)");

    po::positional_options_description p;
    p.add("input", 1);
    p.add("output", 2);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("version")>0)
    {
      std::cout << "mkdssp version " XSSP_VERSION << std::endl;
      exit(0);
    }

    if (vm.count("help") or not vm.count("input"))
    {
      std::cerr << desc << std::endl
         << std::endl
         << "Examples: " << std::endl
         << std::endl
         << "To calculate the secondary structure for the file 1crn.pdb and"
         << std::endl
         << "write the result to a file called 1crn.dssp, you type:"
         << std::endl
         << std::endl
         << "  " << argv[0] << " -i 1crn.pdb -o 1crn.dssp"
         << std::endl
         << std::endl;
#if defined(_MSC_VER)
      std::cerr << std::endl
         << "MKDSSP is a command line application, use the 'Command prompt' "
         << "application" << std::endl
         << "to start " << argv[0] << " You can find the 'Command prompt' in "
         << "the Start menu:" << std::endl
         << std::endl
         << "Start => Accessories => Command prompt" << std::endl
         << std::endl
         << std::endl
         << "Press any key to continue..." << std::endl;
      char ch = _getch();
#endif
      exit(1);
    }

    VERBOSE = vm.count("verbose") != 0;
    if (vm.count("debug"))
      VERBOSE = vm["debug"].as<int>();

    std::string input = vm["input"].as<std::string>();

    std::ifstream infile(input.c_str(),
                         std::ios_base::in | std::ios_base::binary);
    if (not infile.is_open())
      throw std::runtime_error("No such file");

    io::filtering_stream<io::input> in;

#if defined USE_COMPRESSION
    if (ba::ends_with(input, ".bz2"))
    {
      in.push(io::bzip2_decompressor());
      input.erase(input.length() - 4);
    }
    else if (ba::ends_with(input, ".gz"))
    {
      in.push(io::gzip_decompressor());
      input.erase(input.length() - 3);
    }
#endif

    in.push(infile);

    // OK, we've got the file, now create a protein
    MProtein a;

    if (ba::ends_with(input, ".cif"))
      a.ReadmmCIF(in);
    else
      a.ReadPDB(in);

    // then calculate the secondary structure
    a.CalculateSecondaryStructure();

    // and finally report the secondary structure in the DSSP format
    // either to cout or an (optionally compressed) file.
    if (vm.count("output"))
    {
      std::string output = vm["output"].as<std::string>();

      std::ofstream outfile(
          output.c_str(),
          std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
      if (not outfile.is_open())
        throw std::runtime_error("could not create output file");

      io::filtering_stream<io::output> out;
#if defined USE_COMPRESSION
      if (ba::ends_with(output, ".bz2"))
        out.push(io::bzip2_compressor());
      else if (ba::ends_with(output, ".gz"))
        out.push(io::gzip_compressor());
#endif
      out.push(outfile);

      WriteDSSP(a, out);
    }
    else
      WriteDSSP(a, std::cout);
  }
  catch (const std::exception& e)
  {
    std::cerr << "DSSP could not be created due to an error:" << std::endl
       << e.what() << std::endl;
    exit(1);
  }

  return 0;
}

