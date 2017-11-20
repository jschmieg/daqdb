/**
 * Copyright 2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../../include/dht/CChordNode.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <set>

#include "../../include/dht/DhtUtils.h"
#include "ProtocolSingleton.h"

namespace bf = boost::filesystem;
using namespace std;

namespace
{
const string dhtBackBoneIp = "127.0.0.1";
const string dhtOverlayIdentifier = "chordTestBed";
const string rootDirectory = ".";
};

namespace FogKV
{

CChordAdapter::CChordAdapter(as::io_service &io_service, unsigned short port,
			     unsigned short dragonPort, int id)
    : CChordAdapter(io_service, port, dragonPort, id, false)
{
}

CChordAdapter::CChordAdapter(as::io_service &io_service, unsigned short port,
			     unsigned short dragonPort, int id, bool skipShutDown)
    : FogKV::DhtNode(io_service, port, dragonPort), skipShutDown(skipShutDown)
{
	auto dhtPort = FogKV::utils::getFreePort(io_service, port, true);

	string backBone[] = {
		dhtBackBoneIp,
	};

	spNode.reset(P_SINGLETON->initChordNode(
		id, dhtBackBoneIp, dhtPort, dragonPort, dhtOverlayIdentifier,
		rootDirectory));
	spChord.reset(new Node(backBone[0], id, port, dragonPort));

	spNode->join(spChord.get());

	this->setPort(spNode->getThisNode()->getPort());
	this->setDragonPort(spNode->getThisNode()->getDragonPort());
	this->setDhtId(spNode->getThisNode()->getId());
	this->setIp(spNode->getThisNode()->getIp());
}

CChordAdapter::~CChordAdapter()
{
	if (!skipShutDown) {
		spNode->shutDown();
	}
}

std::string
CChordAdapter::printStatus()
{
	return spNode->printStatus();
}

unsigned int
CChordAdapter::getPeerList(boost::ptr_vector<PureNode> &peerNodes)
{
	std::set<unsigned int> addedDhtNodes;
	auto addUniqueNode = [&addedDhtNodes, &peerNodes](Node *pNodeToAdd) {
		if (!addedDhtNodes.count(pNodeToAdd->getId())) {
			peerNodes.push_back(new FogKV::PureNode(
				pNodeToAdd->getIp(), pNodeToAdd->getId(),
				pNodeToAdd->getPort(),
				pNodeToAdd->getDragonPort()));
			addedDhtNodes.emplace(pNodeToAdd->getId());
		}
	};
	addedDhtNodes.emplace(this->getDhtId());

	addUniqueNode(spNode->getPredecessor());
	addUniqueNode(spNode->getSuccessor());

	vector<Node *> nodeFingerTable;
	spNode->getPeerList(nodeFingerTable);
	for (auto pNode : nodeFingerTable) {
		addUniqueNode(pNode);
	}
	return peerNodes.size();
}

void
CChordAdapter::refresh()
{
	spNode->stabilize();
	spNode->fixFingersTable();
	spNode->checkPredecessor();
}

void
CChordAdapter::triggerAggregationUpdate()
{
	//! @todo jradtke Discussion needed how to implement this ...
}

} /* namespace Dht */
