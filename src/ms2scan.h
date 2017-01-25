#ifndef MS2SCAN_H
#define MS2SCAN_H

#include <vector>
#include <string>

struct Query;
#include "peptide.h"
#include "./Scores/CometSearch.h"
//#include "TandemMassSpectrum.h"

#define BIN_RES         1000
#define LOW_BIN_RES     10
//#define TOP_N           5
#define TOPPICKNUM      20
#define ZERO            0.00000001
#define SMALLINCREMENT  0.00000001 // to solve the incorrect cut off 

#define WeightSum 1

using namespace std;

class ProductIon // for each ion
{
	char cIonType;  //y or b
	int iIonNumber;  // 1: y1 or b1; 2: y2 or b2; ...
	int iCharge;     // charge state
	int iMostAbundantPeakIndex; // related to the item with highest intensity
	double dMostAbundantMass;  //related to the item with highest intensity
	double dMostAbundantMZ; // related to the item with highest intensity
	double dMZError;
	double dMassError;  // calculate based on dMZError
	double dScoreWeight; //related to mass error and intensity
	bool bComplementaryFragmentObserved; // if y_k and b_n-k co-exist, where n is # of residues
public:
	ProductIon();
	~ProductIon();
	// set the basic info
	void setProductIon(char cIonTypeInput, int iIonNumberInput, int iChargeInput);
	// set observed info
	void setObservedInfo(double dMZErrorInput, double dWeightInput, double dMostAbundantMZInput,
			int iMostAbundantPeakIndexInput);
	void setComplementaryFragmentObserved(bool bComplementaryFragmentObservedInput);
	char getIonType() {
		return cIonType;
	}
	;
	int getIonNumber() {
		return iIonNumber;
	}
	;
	int getCharge() {
		return iCharge;
	}
	;
	double getMZError() {
		return dMZError;
	}
	;
	double getMassError() {
		return dMassError;
	}
	;
	double getScoreWeight() {
		return dScoreWeight;
	}
	;
	double getMostAbundantMass() {
		return dMostAbundantMass;
	}
	;
	double getMostAbundantMZ() {
		return dMostAbundantMZ;
	}
	;
	int getMostAbundantPeakIndex() {
		return iMostAbundantPeakIndex;
	}
	;
	bool getComplementaryFragmentObserved() {
		return bComplementaryFragmentObserved;
	}
	;
};

class PeakList{
public:
	int iLowestMass;
	// excluded
	int iHighestMass;
	int * pMassHub;
	double * pPeaks;
	char * pClasses;
	int iPeakSize;
	int iMassHubSize;
	int iMassHubSizeMinorOne;
	static char iNULL;

	PeakList(map<double, char> * _peakData);
	~PeakList();

	char end();
	char findNear(double mz, double tolerance);
	int size();
};

class PeptideUnit {
public:
	double dCalculatedParentMass;
	double dScore;
	string sIdentifiedPeptide;
	string sPeptideForScoring;
	string sOriginalPeptide;
	string sProteinNames;
	string sScoringFunction;
	char cIdentifyPrefix;
	char cIdentifySuffix;
	char cOriginalPrefix;
	char cOriginalSuffix;
	double dPepMass;
	int iPepLen;
	int iEnzInt;

	vector<double> vdScores;
	static size_t iNumScores;
	vector<double> vdRank;
	vector<double> vdFraction;

	bool addScores(double dScore);
	int getNumProtein();
	bool isDecoy(string & _sDecoyPrefix);
	void setPeptideUnitInfo(const Peptide * currentPeptide, const double & dScore, string sScoringFunction);
};

class MS2Scan {
	int bin_res, iMaxMZ, iMinMZ;

	double dMassTolerance; //<FRAGMENT_IONS>
	double dProtonMass; // proton mass

	string sScanType; //format: FT-MS1/FT-MS2@CID

	vector<double> vdpreprocessedMZ;
	vector<double> vdpreprocessedIntensity;
	vector<int> vipreprocessedCharge;  // the value of vipreprocessedCharge is zero for low-resolution MS2Scan

	// vdMaxMzIntensity[i] is the maximum intensity at M/Z window of i plus and minus iMzRange
	vector<double> vdMaxMzIntensity;
	vector<double> vdMzIntensity;
	vector<double> vdHighIntensity; //thresholds

	vector<int> vbPeakPresenceBins;
	vector<pair<int, int> > vbPeakPresenceBins2D; // first is lower bounder, second is upper bound
	vector<int> viIntensityRank; // ranks of preprocessed intensities staring with 0

	void preprocessLowMS2();
	void preprocessHighMS2();
	void initialPreprocess();
	int getMaxValueIndex(const vector<double> & vdData);
	void normalizeMS2scan();
	void setIntensityThreshold();
	void filterMS2scan();
	static bool mygreater(double i, double j);
	void binCalculation2D(); // replace binCalculation();
	static bool GreaterScore(PeptideUnit * p1, PeptideUnit * p2);
	void WeightCompare(const string & sPeptide, vector<bool> & vbFragmentZ2);
	bool searchMZ(const double & dTarget, int & iIndex4Found); // corresponds to binCalculation()
	bool searchMZ2D(const double & dTarget, int & iIndex4Found); // corresponds to binCalculation2D()
	bool searchMZ2D(const double & dTarget, const double & dErrRange, int & iIndex4Found);
	void sortPreprocessedIntensity(); //sort preprocessed Intensity;

	void cleanup();

	bool binarySearch(const double & dTarget, const vector<double> & vdList, const double & dTolerance,
			vector<int> & viIndex4Found);

	// build the map between y or b ions for observed intensity and related mass (only for one ion)
	bool findProductIon(const vector<double> & vdIonMass,   // expected mass
			const vector<double> & vdIonProb,
			//expected intensity based on the summation of all related intensities
			const int & iCharge, double & dScoreWeight, double & dAverageMZError, double & dMostAbundantObservedMZ, // with highest intensity
			int & iMostAbundantPeakIndex); // start with 0

	bool findProductIonSIP(const vector<double> & vdIonMass,   // expected mass
			const vector<double> & vdIonProb,
			//expected intensity based on the summation of all related intensities
			const int & iCharge, double & dScoreWeight, double & dAverageMZError, double & dMostAbundantObservedMZ, // with highest intensity
			int & iMostAbundantPeakIndex); // start with 0
public:
	MS2Scan();
	~MS2Scan();

	// FT2file name
	string sFT2Filename;
	// Parent ion charge state
	int iParentChargeState;
	// Parent ion M/Z
	double dParentMZ;
	// Parent neutral mass
	double dParentNeutralMass;
	// Parent mass with charge
	double dParentMass;
	// Product ions in the scan
	int iScanId;
	// peak list of mass over charge
	vector<double> vdMZ;
	// peak list of intensity
	vector<double> vdIntensity;
	// the value of viCharge is zero for low-resolution MS2
	vector<int> viFragmentCharges;
	// Is the MS1 scan a high-resolution scan?
	bool isMS1HighRes;
	// Is this MS2 scan a high-resolution scan?
	bool isMS2HighRes;
    //current set of peptides to be scored
	vector<Peptide *> vpPeptides;

	// Peptides associated with this scan
	vector<PeptideUnit *> vpTopPeptides;
	// the scores of all scored peptides
	vector<double> vdPeptideScores;
	// number of peptide in the list
	int iSizeOfTopPeptides;
	// WDP scores
	double dSumWeightSumScore;
	double dSumSquareWeightSumScore;

	 // false when pre-process fail on bad data
	bool bSetMS2Flag;

	string getRTime();
	string getScanType();
	//merge same peptide. If no same peptide return false, otherwise, return true
	bool mergePeptide(vector<PeptideUnit*>& vpTopPeptides, const string & sPeptide, const string & sProteinName);
	// preprocess this scan, including
	// (1) remove noise peaks
	// (2) normalize intensity
	void preprocess();
	// normalize raw scores
	void postprocess();
	void saveScore(const double & dScore, const Peptide * currentPeptide, vector<PeptideUnit *> & vpTopPeptides,
			vector<double> & vdAllScores, string sScoreFunction = "WeightSum");
	void setRTime(string _sRTime);
	void setScanType(string sScanType);
	// scoring functions
	double scoreWeightSum(string * currentPeptide, vector<double> * pvdYionMass, vector<double> * pvdBionMass);
	// void scoreRankSum_test(Peptide * currentPeptide);
	double scoreWeightSumHighMS2(string * currentPeptide, vector<vector<double> > * vvdYionMass,
			vector<vector<double> > * vvdYionProb, vector<vector<double> > * vvdBionMass,
			vector<vector<double> > * vvdBionProb);
	void sortPeakList(); // bubble sort the peak list by MZ in ascending order


	//-----------Comet Begin-------------
	struct Query * pQuery;
	//-----------Comet End---------------
	//-----------Myrimatch Begin-------------
	bool bSkip;
	map<double, char> * peakData;
	PeakList * pPeakList;
	vector<int> * intenClassCounts;
	double mvh;
	int totalPeakBins;
	double mzLowerBound;
	double mzUpperBound;
	//-----------Myrimatch End---------------
	//-----------Features--------------------
	double dSumIntensity;
	double dMaxIntensity;
	void sumIntensity();
	void scoreFeatureCalculation();
	int iNumPeptideAssigned;
	int getMaxNumProteinPsm();
	bool isAnyScoreInTopN(size_t _iIndexPeptide, double _dLogRank);
	string sRTime;
	//-----------Features End----------------

};

#endif // MS2SCAN_H
