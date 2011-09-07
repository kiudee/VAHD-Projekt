#ifndef NODE_H_
#define NODE_H_

#include <string>
#include <unordered_map>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Supervisor.h"
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
    HashMap data; //was soll gespeichert werden?

    void checkDead(NodeRelay *side);
    void checkValid();
    double h(int kk);
    double g(int kk);
    double calcRoutingBound();


public:
    FirstAction(Node,Init)

    Action Init(InitObj *init);
    Action ConnectChild(IdObj *id);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn();
    Action Insert(InsertObj *iob);
    Action Delete(NumObj *key);
    Action LookUp(KeyObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
    Action Probing(Probe *ido);
    Action BuildList(IdObj *id);
    Action BuildWeakConnectedComponent(NumObj *numo);
    Action ReceiveLookUp(DateObj *dob);
};


#endif  // NODE_H_
