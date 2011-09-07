#ifndef NODE_H_
#define NODE_H_
#include <unordered_map>

SubjectType(Node)
{
protected:
    NodeRelay *left, *right;//, *node0, *node1;
    Relay *node0, *node1;
    Relay *in;
    double num;
    bool isReal;
    std::unordered_map<int, Object> data;//was soll gespeichert werden?

    void checkDead(NodeRelay *side);
    void checkValid();
    double h(int kk);
    double g(int kk);


public:
    FirstAction(Node,Init)

    Action Init(NumObj *con);
    Action Init(DoubleObj *con);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn();
    Action Insert(InsertObj *iob);
    Action Delete(NumObj *key);
    Action LookUp(NumObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
    Action Probing(Probe *ido);
    Action BuildList(IdObj *id);
    Action BuildWeakConnectedComponent(NumObj *numo);
    Action ReceiveLookUp(DateObj *dob);
};


#endif  // NODE_H_
