#ifndef NODE_H_
#define NODE_H_

SubjectType(Node)
{
protected:
    NodeRelay *left, *right, *node0, *node1;
    Relay *in;
    double num;
    bool isReal;
    std::unordered_map<int, Object> data;//was soll gespeichert werden?

    void checkDead(NodeRelay *side);
    void checkValid();
    void mult_hash();


public:
    FirstAction(Node,Init)

    Action Init(ConObj *con);
    Action RegisterChild(IdObj *id);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn();
    Action Insert(DateObj *num);
    Action Delete(DoubleObj *key);
    Action LookUp(DoubleObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
    Action Probing(Probe *id);
    Action BuildList(IdObj *id);
    Action BuildWeakConnectedComponent(IdObj *id);
};


#endif  // NODE_H_
