#ifndef __LEDGERACQUIRE__
#define __LEDGERACQUIRE__

#include <vector>
#include <map>
#include <list>

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "Ledger.h"
#include "Peer.h"
#include "TaggedCache.h"
#include "InstanceCounter.h"
#include "ripple.pb.h"

DEFINE_INSTANCE(PeerSet);

class PeerSet : private IS_INSTANCE(PeerSet)
{
protected:
	uint256 mHash;
	int mTimerInterval, mTimeouts;
	bool mComplete, mFailed, mProgress;

	boost::recursive_mutex					mLock;
	boost::asio::deadline_timer				mTimer;
	boost::unordered_map<uint64, int>		mPeers;

	PeerSet(const uint256& hash, int interval);
	virtual ~PeerSet() { ; }

	void sendRequest(const ripple::TMGetLedger& message);
	void sendRequest(const ripple::TMGetLedger& message, Peer::ref peer);

public:
	const uint256& getHash() const		{ return mHash; }
	bool isComplete() const				{ return mComplete; }
	bool isFailed() const				{ return mFailed; }
	int getTimeouts() const				{ return mTimeouts; }

	void progress()						{ mProgress = true; }

	void peerHas(Peer::ref);
	void badPeer(Peer::ref);
	void resetTimer();

	int takePeerSetFrom(const PeerSet& s);
	int getPeerCount() const;

protected:
	virtual void newPeer(Peer::ref) = 0;
	virtual void onTimer(bool progress) = 0;
	virtual boost::weak_ptr<PeerSet> pmDowncast() = 0;

	void setComplete()					{ mComplete = true; }
	void setFailed()					{ mFailed = true; }
	void invokeOnTimer();

private:
	static void TimerEntry(boost::weak_ptr<PeerSet>, const boost::system::error_code& result);
};

class LedgerAcquire : public PeerSet, public boost::enable_shared_from_this<LedgerAcquire>
{ // A ledger we are trying to acquire
public:
	typedef boost::shared_ptr<LedgerAcquire> pointer;

protected:
	Ledger::pointer mLedger;
	bool mHaveBase, mHaveState, mHaveTransactions, mAborted, mSignaled, mAccept;

	std::vector< boost::function<void (LedgerAcquire::pointer)> > mOnComplete;

	void done();
	void onTimer(bool progress);

	void newPeer(Peer::ref peer) { trigger(peer, false); }

	boost::weak_ptr<PeerSet> pmDowncast();

public:
	LedgerAcquire(const uint256& hash);
	virtual ~LedgerAcquire()			{ ; }

	bool isBase() const					{ return mHaveBase; }
	bool isAcctStComplete() const		{ return mHaveState; }
	bool isTransComplete() const		{ return mHaveTransactions; }
	Ledger::pointer getLedger()			{ return mLedger; }
	void abort()						{ mAborted = true; }
	bool setAccept()					{ if (mAccept) return false; mAccept = true; return true; }

	void addOnComplete(boost::function<void (LedgerAcquire::pointer)>);

	bool takeBase(const std::string& data);
	bool takeTxNode(const std::list<SHAMapNode>& IDs, const std::list<std::vector<unsigned char> >& data);
	bool takeTxRootNode(const std::vector<unsigned char>& data);
	bool takeAsNode(const std::list<SHAMapNode>& IDs, const std::list<std::vector<unsigned char> >& data);
	bool takeAsRootNode(const std::vector<unsigned char>& data);
	void trigger(Peer::ref, bool timer);
	bool tryLocal();
	void addPeers();
};

class LedgerAcquireMaster
{
protected:
	boost::mutex mLock;
	std::map<uint256, LedgerAcquire::pointer> mLedgers;

public:
	LedgerAcquireMaster() { ; }

	LedgerAcquire::pointer findCreate(const uint256& hash);
	LedgerAcquire::pointer find(const uint256& hash);
	bool hasLedger(const uint256& ledgerHash);
	void dropLedger(const uint256& ledgerHash);
	bool gotLedgerData(ripple::TMLedgerData& packet, Peer::ref);
};

#endif

// vim:ts=4