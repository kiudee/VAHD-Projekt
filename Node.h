#ifndef NODE_H_
#define NODE_H_

SubjectType(Node)
{
protected:
    Relay *left, *right, *node0, *node1;
    Relay *in;
    int id;
    bool isReal;

public:
    FirstAction(Node,Init)

    Action Init(ConObj *con);
    Action RegisterChiled(IdObj *id);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn(IdObj *id);
    Action Insert(NumObj *num);
    Action Delete(NumObj *key);
    Action LookUp(NumObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
};

Action Node::Init(ConObj *con)
{
    // Unpack constructor object:
    id = con->position;
    isReal = con->isReal;
    delete con;

    // Initialize pointers and relays
    left = NULL;
    right = NULL;
    in = new Relay;
    // Connect to supervisor or real node:
    if (isReal) {
        idp = new IdPair(new IdObj(id, new Identity(in)),
                         new IdObj(id, new Identity(in)));
        parent->call(Supervisor::SetLink, idp);
    } else {
        ido = new IdObj(id, new Identity(in));
        parent->call(Node::RegisterChild, ido);
    }
}

Action Node::Wakeup(NumObj *num)
{
    // Periodically activate BuildDeBruijn:
    if (num->num == 0) {
        BuildDeBruijn(NULL);
    } else {
        num->num--;
        call(Node::Wakeup, num);
    }
}

Action Node::BuildDeBruijn(IdObj *id)
{

}

Action Node::Insert(NumObj *num)
{

}

Action Node::Delete(NumObj *key)
{

}

Action Node::LookUp(NumObj *key)
{

}

Action Node::Join(IdObj *id)
{

}


Action Node::Leave(IdObj *id)
{

}

#endif  // NODE_H_
