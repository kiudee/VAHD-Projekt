#ifndef NODE_H_
#define NODE_H_
#include <unordered_map>
#include "Subjects1-6.h"
#include "Hash.h"

typedef std::unordered_map<int, std::string> HashMap;


SubjectType(Node)
{
protected:
    NodeRelay *left, *right;//, *node0, *node1;
    Relay *node0, *node1;
    Relay *in;
    double num;
    bool isReal;
    int leftstable;
    int rightstable;
    HashMap data;//was soll gespeichert werden?

    void checkDead(NodeRelay *side);
    void checkValid();
    double h(int kk);
    double g(int kk);
    double calcRoutingBound();
    int checkStable();
    int isStable(); //TODO might be useless


public:
    FirstAction(Node,Init)

    Action Init(InitObj *init);
    Action ConnectChild(IdObj *id);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn();
    Action Insert(DateObj *dob);
    Action Delete(NumObj *key);
    Action LookUp(NumObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
    Action Probing(Probe *ido);
    Action BuildList(IdObj *id);
    Action BuildWeakConnectedComponent(NumObj *numo);
    Action ReceiveLookUp(DateObj *dob);
    Action Search(SearchJob *sj);
    Action TriggerDataTransfer(IdObj *ido);
};


#endif  // NODE_H_
