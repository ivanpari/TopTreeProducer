#include "../interface/JetAnalyzer.h"

using namespace std;
using namespace TopTree;
using namespace reco;
using namespace edm;

JetAnalyzer::JetAnalyzer():verbosity_(0),useMC_(false)
{
}

JetAnalyzer::JetAnalyzer(int verbosity):verbosity_(verbosity),useMC_(false)
{
}

JetAnalyzer::JetAnalyzer(const edm::ParameterSet& myConfig, int verbosity):verbosity_(verbosity)
{
	useMC_ = myConfig.getUntrackedParameter<bool>("doJetMC");
}

JetAnalyzer::~JetAnalyzer()
{
}

bool Rsortrule (std::pair <double,double> p1, std::pair <double,double> p2 )
{
	return p1.second<p2.second; 
}

TRootJet JetAnalyzer::Process(const reco::Jet* jet, std::string dataType)
{

	TRootJet localJet(
		jet->px()
		,jet->py()
		,jet->pz()
		,jet->energy()
		,jet->vx()
		,jet->vy()
		,jet->vz()
		,jet->pdgId()
		,jet->charge()
	); 

	localJet.setNConstituents(jet->nConstituents());
	localJet.setJetArea(jet->jetArea()); 
	localJet.setPileupEnergy(jet->pileup());
	localJet.setMaxDistance(jet->maxDistance());

	if( dataType=="PATAOD" || dataType=="PAT" )
	{
		// Some specific methods to pat::Jet
		const pat::Jet *patJet = dynamic_cast<const pat::Jet*>(&*jet);

		localJet.setJetArea(jet->jetArea());
		localJet.setPileupEnergy(jet->pileup());
		localJet.setMaxDistance(jet->maxDistance());

		// Variables from pat::Jet (Basic)
		localJet.setBtag_jetBProbabilityBJetTags(patJet->bDiscriminator("jetBProbabilityBJetTags"));
		localJet.setBtag_jetProbabilityBJetTags(patJet->bDiscriminator("jetProbabilityBJetTags"));
		localJet.setBtag_trackCountingHighPurBJetTags(patJet->bDiscriminator("trackCountingHighPurBJetTags"));
		localJet.setBtag_trackCountingHighEffBJetTags(patJet->bDiscriminator("trackCountingHighEffBJetTags"));
		localJet.setBtag_simpleSecondaryVertexHighEffBJetTags(patJet->bDiscriminator("simpleSecondaryVertexHighEffBJetTags"));
		localJet.setBtag_simpleSecondaryVertexHighPurBJetTags(patJet->bDiscriminator("simpleSecondaryVertexHighPurBJetTags"));
		localJet.setBtag_combinedSecondaryVertexBJetTags(patJet->bDiscriminator("combinedSecondaryVertexBJetTags"));
		localJet.setBtag_combinedSecondaryVertexMVABJetTags(patJet->bDiscriminator("combinedSecondaryVertexMVABJetTags"));
		localJet.setBtag_combinedMVABJetTags(patJet->bDiscriminator("combinedMVABJetTags"));
		localJet.setBtag_softMuonBJetTags(patJet->bDiscriminator("softMuonBJetTags"));
		localJet.setBtag_softMuonByPtBJetTags(patJet->bDiscriminator("softMuonByPtBJetTags"));
		localJet.setBtag_softMuonByIP3dBJetTags(patJet->bDiscriminator("softMuonByIP3dBJetTags"));
		localJet.setBtag_softElectronBJetTags(patJet->bDiscriminator("softElectronBJetTags"));
		localJet.setBtag_softElectronByPtBJetTags(patJet->bDiscriminator("softElectronByPtBJetTags"));
		localJet.setBtag_softElectronByIP3dBJetTags(patJet->bDiscriminator("softElectronByIP3dBJetTags"));
		localJet.setBtag_ghostTrackBJetTags(patJet->bDiscriminator("ghostTrackBJetTags"));
		
		/*cout << "******** BTAGGING INFO *******" << endl;
		cout << "jetBProbabilityBJetTags -> " << patJet->bDiscriminator("jetBProbabilityBJetTags") << endl;
		cout << "jetProbabilityBJetTags -> " << patJet->bDiscriminator("jetProbabilityBJetTags") << endl;
		cout << "simpleSecondaryVertexHighEffBJetTags -> " << patJet->bDiscriminator("simpleSecondaryVertexHighEffBJetTags") << endl;
		cout << "simpleSecondaryVertexHighPurBJetTags -> " << patJet->bDiscriminator("simpleSecondaryVertexHighPurBJetTags") << endl;
		cout << "trackCountingHighEffBJetTags -> " << patJet->bDiscriminator("trackCountingHighEffBJetTags") << endl;
		cout << "trackCountingHighPurBJetTags -> " << patJet->bDiscriminator("trackCountingHighPurBJetTags") << endl;
		cout << "combinedSecondaryVertexBJetTags -> " << patJet->bDiscriminator("combinedSecondaryVertexBJetTags") << endl;	
		cout << "combinedSecondaryVertexMVABJetTags -> " << patJet->bDiscriminator("combinedSecondaryVertexMVABJetTags") << endl;
		cout << "softMuonBJetTags -> " << patJet->bDiscriminator("softMuonBJetTags") << endl;
		cout << "softElectronBJetTags -> " << patJet->bDiscriminator("softElectronBJetTags") << endl;*/

		// Use  associated tracks to calculate charged broadness of the jet
		// FIXME - Check generalTracks collection is present
		reco::TrackRefVector tracks =  patJet->associatedTracks() ;
		Float_t deltaR = 0.;
		Float_t pTrackerTot = 0.;
		// TODO - Use std::map....
		vector < pair < Float_t , Float_t > > tracksVPair ;
		pair < Float_t , Float_t > tracksPair;

		for(unsigned int iTracks = 0; iTracks< patJet->associatedTracks().size() ; iTracks++)
		{
			deltaR = localJet.DeltaR(TLorentzVector(tracks[iTracks]->px(),tracks[iTracks]->py(),tracks[iTracks]->pz(),0));
			pTrackerTot += tracks[iTracks]->p();
			tracksPair.first = tracks[iTracks]->p();
			tracksPair.second = deltaR;
			tracksVPair.push_back(tracksPair);

			//cout << "DeltaR " << deltaR << endl;

		}
	
		sort(tracksVPair.begin(), tracksVPair.end(), Rsortrule);
		Float_t Rmin = 0;
		Float_t pDummy = 0;
		for(std::vector<std::pair< Float_t,Float_t > >::iterator i = tracksVPair.begin(); i!=tracksVPair.end(); i++)
		{
			pDummy+=(*i).first;
			if (pDummy>0.75*(pTrackerTot))
			{
				Rmin = (*i).second;
				break;
			}
		}
		//cout << "Rmin " << Rmin << endl;
		if (Rmin<1e-5) {Rmin=0.;}
		localJet.setChargedBroadness(Rmin);

		// jet correction factors

		pat::Jet rawJet = patJet->correctedJet("raw");

		localJet.setJetCorrFactor(0,"L1",rawJet.jecFactor("off"));
		localJet.setJetCorrFactor(1,"L1L2",rawJet.jecFactor("rel"));
		localJet.setJetCorrFactor(2,"L1L2L3",rawJet.jecFactor("abs"));
		localJet.setJetCorrFactor(3,"L1L2L3L4",rawJet.jecFactor("emf"));

		localJet.setJetCorrFactor(4,"L1L2L3L4L5_glu",rawJet.jecFactor("had","glu"));
		localJet.setJetCorrFactor(5,"L1L2L3L4L5_uds",rawJet.jecFactor("had","uds"));
		localJet.setJetCorrFactor(6,"L1L2L3L4L5_c",rawJet.jecFactor("had","c"));
		localJet.setJetCorrFactor(7,"L1L2L3L4L5_b",rawJet.jecFactor("had","b"));

		localJet.setJetCorrFactor(8,"L1L2L3L4L5L6_glu",rawJet.jecFactor("ue","glu"));
		localJet.setJetCorrFactor(9,"L1L2L3L4L5L6_uds",rawJet.jecFactor("ue","uds"));
		localJet.setJetCorrFactor(10,"L1L2L3L4L5L6_c",rawJet.jecFactor("ue","c"));
		localJet.setJetCorrFactor(11,"L1L2L3L4L5L6_b",rawJet.jecFactor("ue","b"));

		localJet.setJetCorrFactor(12,"L1L2L3L4L5L6L7_glu",rawJet.jecFactor("part","glu"));
		localJet.setJetCorrFactor(13,"L1L2L3L4L5L6L7_uds",rawJet.jecFactor("part","uds"));
		localJet.setJetCorrFactor(14,"L1L2L3L4L5L6L7_c",rawJet.jecFactor("part","c"));
		localJet.setJetCorrFactor(15,"L1L2L3L4L5L6L7_b",rawJet.jecFactor("part","b"));

		//cout << "Abs " << patJet->jecFactor("raw") << endl; 
		// Matched genParticle
		if (useMC_)
		{
			// MC truth associator index
			if ((patJet->genParticleRef()).isNonnull()) {
				localJet.setGenParticleIndex((patJet->genParticleRef()).index());
			} else {
				localJet.setGenParticleIndex(-1);
			}

			// set the parton flavour
			localJet.setPartonFlavour(patJet->partonFlavour());

			// check if jet comes from a top
			// change that ... 
			// look on TtGenEvent
			// keep the TRef ...
			bool IsTopJet =  false;
			if(patJet->genParton())
			{
				const reco::Candidate* gen = patJet->genParton();
				while(gen->mother())
				{
					if(abs((gen->mother())->pdgId()) == 6)
					{
						IsTopJet =  true;
						break;
					}
					else
					{
						gen = (gen->mother() );
					}
				}
			}
			localJet.setIsTopJet(IsTopJet);
		}

	}

	return localJet;
}
