#ifndef NODE_H_
#define NODE_H_

#include <math.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Supervisor.h"
#include "Hash.h"

typedef std::unordered_map<int, DATATYPE> HashMap;

SubjectType(Node)
{
protected:
    bool leftstable;
    bool rightstable;
    bool node0left;
    bool node1left;
    HashMap data;//for a datatransfer another data structure would be better e.g. a binary tree.
    std::shared_ptr<std::ofstream> csvFile;

    // BuildDeBruijn:
    void checkDead(NodeRelay **side, bool right);
    void checkValid();
    double calcRoutingBound();
    void checkStable(double id);
    void BuildSide(IdObj * ido, NodeRelay **side, bool right);
    bool isSelf(IdObj * ido);

    // Search:
    bool doLastRoutingPhase(SearchJob * sj);
    bool doDebruijnHop(SearchJob * sj);
    void doListHop(SearchJob * sj);
    void delegateSearchJobToLastNode(SearchJob * sj);


public:
    // Needed for Supervisor output:
    double num;
    bool isReal;
    Relay *in;
    NodeRelay *left, *right;//, *node0, *node1;
    NodeRelay *node0, *node1;

    FirstAction(Node, Init)

    Action Init(InitObj * init);
    Action ConnectChild(IdObj * id);
    Action Wakeup(NumObj * num);
    Action BuildDeBruijn();
    Action Insert(DateObj * dob);
    Action DataInsert(DateObj * dob);
    Action Delete(NumObj * key);
    Action LookUp(NumObj * key);
    Action Join(IdObj * id);
    Action Leave();
    Action VirtualNodeLeave();
    Action Probing(Probe * ido);
    Action BuildList(IdObj * id);
    Action BuildWeakConnectedComponent(NumObj * numo);
    Action ReceiveLookUp(DateObj * dob);
    Action FinishSearch(SearchJob * sj);
    Action Search(SearchJob * sj);
    Action TriggerDataTransfer(IdObj * ido);
    Action RemoveVirtualChild(DoubleObj * dob);

    Action _DebugRouteFromLeftToRight(NumObj * numo);
    Action _DebugRouteFromRightToLeft();
};


#endif  // NODE_H_
