// maxhom version of hssp generating code
//
//	Copyright, M.L. Hekkelman, UMC St. Radboud, Nijmegen
//

#pragma once

#include <iostream>
#include <vector>

#include <boost/filesystem/path.hpp>

class MProtein;

namespace hmmer
{
	
void CreateHSSP(
	CDatabankPtr					inDatabank,
	MProtein&						inProtein,
	const boost::filesystem::path&	inFastaDir,
	const boost::filesystem::path&	inJackHmmer,
	uint32							inIterations,
	uint32							inMinSeqLength,
	std::ostream&					outHSSP);

void CreateHSSP(
	CDatabankPtr					inDatabank,
	const std::string&				inProtein,
	const boost::filesystem::path&	inFastaDir,
	const boost::filesystem::path&	inJackHmmer,
	uint32							inIterations,
	std::ostream&					outHSSP);

}
