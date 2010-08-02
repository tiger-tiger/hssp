// substitution matrix code

#include "matrix.h"

#include <sstream>
#include <iostream>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "../matrices/matrices.h"

using namespace std;
namespace io = boost::iostreams;

substitution_matrix::substitution_matrix(const string& name)
	: m_matrix(sizeof(kAA), sizeof(kAA))
{
	if (name == "BLOSUM80")
	{
		io::stream<io::array_source> in(kBLOSUM80, strlen(kBLOSUM80));
		read(in);
	}
	else if (name == "BLOSUM62")
	{
		io::stream<io::array_source> in(kBLOSUM62, strlen(kBLOSUM62));
		read(in);
	}
	else if (name == "BLOSUM45")
	{
		io::stream<io::array_source> in(kBLOSUM45, strlen(kBLOSUM45));
		read(in);
	}
	else if (name == "BLOSUM30")
	{
		io::stream<io::array_source> in(kBLOSUM30, strlen(kBLOSUM30));
		read(in);
	}
	else if (name == "GONNET250")
	{
		io::stream<io::array_source> in(kGONNET250, strlen(kGONNET250));
		read(in);
	}
	else
		throw my_bad(boost::format("unsupported matrix %1%") % name);
}

substitution_matrix::substitution_matrix(
	const substitution_matrix& m, bool positive)
	: m_matrix(sizeof(kAA), sizeof(kAA))
{
	int8 min = 0;
	
	for (uint32 y = 0; y < kAACount; ++y)
	{
		for (uint32 x = 0; x < kAACount; ++x)
		{
			m_matrix(x, y) = m.m_matrix(x, y);
			
			if (min > m_matrix(x, y))
				min = m_matrix(x, y);
		}
	}
	
	if (min < 0)
	{
		min = -min;
		
		for (uint32 y = 0; y < kAACount; ++y)
		{
			for (uint32 x = 0; x <= y; ++x)
			{
				m_matrix(x, y) += min;
				if (x != y)
					m_matrix(y, x) += min;
			}
		}
		
		float sum = 0;
		for (uint32 ry = 1; ry < 20; ++ry)
		{
			for (uint32 rx = 0; rx < ry; ++rx)
				sum += m_matrix(rx, ry);
		}
		
		m_mismatch_average = sum / ((20 * 19) / 2);
	}
}

void substitution_matrix::read(istream& is)
{
	sequence ix;
	
	// first read up until we've got the header and calculate the index
	for (;;)
	{
		string line;
		getline(is, line);
		if (line.empty())
		{
			if (is.eof())
				break;
			continue;
		}
		if (line[0] == '#')
			continue;
		
		if (line[0] != ' ')
			throw my_bad("invalid matrix file");
		
		string h;
		foreach (char ch, line)
		{
			if (ch != ' ')
				h += ch;
		}
		
		ix = encode(h);
		
		break;
	}
	
	for (;;)
	{
		string line;
		getline(is, line);
		if (line.empty())
		{
			if (is.eof())
				break;
			continue;
		}
		if (line[0] == '#')
			continue;
		
		uint32 row = encode(line.substr(0, 1))[0];
		
		stringstream s(line.substr(1));
		int32 v;

		for (uint32 i = 0; i < ix.length(); ++i)
		{
			s >> v;
			m_matrix(row, ix[i]) = v;
		}
	}
	
	// calculate mismatch_average
	
	float sum = 0;
	for (uint32 ry = 1; ry < 20; ++ry)
	{
		for (uint32 rx = 0; rx < ry; ++rx)
			sum += m_matrix(rx, ry);
	}
	
	m_mismatch_average = sum / ((20 * 19) / 2);
}

// --------------------------------------------------------------------

substitution_matrix_family::substitution_matrix_family(
	const std::string& name)
{
	if (name != "BLOSUM")
		throw my_bad(boost::format("unsuppported matrix %1%") % name);

	m_smat[0] = new substitution_matrix(name + "80");
	m_smat[1] = new substitution_matrix(name + "62");
	m_smat[2] = new substitution_matrix(name + "45");
	m_smat[3] = new substitution_matrix(name + "30");

	m_pos_smat[0] = new substitution_matrix(*m_smat[0], true);
	m_pos_smat[1] = new substitution_matrix(*m_smat[1], true);
	m_pos_smat[2] = new substitution_matrix(*m_smat[2], true);
	m_pos_smat[3] = new substitution_matrix(*m_smat[3], true);
}

substitution_matrix_family::~substitution_matrix_family()
{
	delete m_smat[0];
	delete m_smat[1];
	delete m_smat[2];
	delete m_smat[3];

	delete m_pos_smat[0];
	delete m_pos_smat[1];
	delete m_pos_smat[2];
	delete m_pos_smat[3];
}