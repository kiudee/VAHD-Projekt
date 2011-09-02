SubjectType(Node)
{
    protected:
    Relay *left, *right, *node0, *node1;

    public:
    FirstAction(Node,Init)
        Action Init(ConObj *con);

    Action Wakeup(NumObj *num);
    Action Insert(NumObj *num);
    Action Delete(NumObj *key);
    Action LookUp(NumObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
};
