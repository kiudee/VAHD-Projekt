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
    idp = new IdPair(new IdObj(id, new Identity(in)),
                     new IdObj(id, new Identity(in)));
    // Connect to supervisor:
    parent->call(Supervisor::SetLink, idp);
}

Action Node::Wakeup(NumObj *num)
{

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
