#ifndef NODE_H_
#define NODE_H_

SubjectType(Node)
{
protected:
    NodeRelay *left, *right, *node0, *node1;
    Relay *in;
    int id;
    double value;
    bool isReal;

    void checkDead(NodeRelay *side);
    void checkValid();


public:
    FirstAction(Node,Init)

    Action Init(ConObj *con);
    Action RegisterChild(IdObj *id);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn(IdObj *id);
    Action Insert(NumObj *num);
    Action Delete(NumObj *key);
    Action LookUp(NumObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
};


#endif  // NODE_H_
