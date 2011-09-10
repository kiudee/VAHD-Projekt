#ifndef SUBJECTS1_6_H_
#define SUBJECTS1_6_H_
// **************************************************************
// *                                                            *
// *  Subjects.h  --  Version 1.6                               *
// *                                                            *
// *  (C) Christian Scheideler, 2011                            *
// *                                                            *
// **************************************************************

/*
Please do not define any variables or functions that start with '_' so
that there are no conflicts with the Subjects environment.

There are three basic data types: Subject, Relay and Object.

- Subject: base class for subjects. Subjects are atomic threads with their
  own private data. All activities (action executions) are done by subjects.
  To prevent any resource issues, subjects are only allowed to have static
  data.

- Relay: class for ports that allow subjects to establish connections and
  exchange objects.

- Object: base class for objects. Objects are containers of dynamic data and
  MUST be strictly passive, i.e., no actions should be defined for them.
  Every object should only have one owner at a time (although it is difficult
  to enforce in C++). That is, each time an object is passed from subject A
  to subject B, the ownership is also transferred from A to B, and A should
  have no access any more to the object.
  Ideally, it should not be possible to copy an object via its reference
  (but only by explicitly copying its contents), and variables representing
  objects should not be able to point to the same object (though this is
  difficult to enforce in C++).

  There are two predefined subclasses of objects.

  - Identity: contains relay point identification info.

  - Cocoon: contains a dormant subject.

  Identities and Cocoons are so-called dark objects, i.e., they cannot be
  inspected by the application but can only be used (once!) as arguments in
  calls.

  User-defined objects should be defined as

  ObjectType(<UserObject>)
  {
  public:
    <user-defined variables>
    <possibly constructor>
  };

  User-defined subjects should be defined as

  SubjectType(<UserSubject>)
  {
  protected:
    <user-defined variables>
    <internal user-defined methods>

  public:
    FirstAction(<UserSubject>,<Action>)

    Action <Action>() {...}
    Action <Action>(<UserObject> *o) {...}
  };

  All remotely callable user-defined actions have to be of the form as
  specified under "public".


Starting the Subjects environment:

  In main(), type

  runSubjects(<UserSubject>, <UserObject> *o | NONE, t);

  where t can be any unsigned long value. If t=0, then the Subjects
  environment will run until there is no more request to process. Otherwise,
  the environment runs for t rounds.


Identity commands:

- new Identity(Relay *r): this creates a public identity of a local relay
  point r.

- new Identity(Relay *r, Relay *r'): this creates a private identity of
  a local relay point r for the base of relay point r'.

- delete i: this deletes a given identity i


Cocoon commands:

- Cocoon *publicCocoon(<UserSubject>, <UserObject> *o | NONE): this creates a
  public cocoon of type UserSubject.

- Cocoon *privateCocoon(<UserSubject>, <UserObject> *o | NONE, Relay *r): this
  creates a cocoon of the given subject for the base of relay point r.

- delete c: this deletes a given cocoon c


Reserved subject variables (ONLY accessible within a subject):

- unsigned long _debugID: identification number of the subject
  (Subjects by themselves do not have identities, but this is useful for
  debugging purposes.)

- Relay *parent: this is a relay point with a connection to the subject that
  spawned the given subject. It is set automatically by the platform once
  the constructor has been completed.

- unsigned long source: this contains the ID of the subject that created the
  currently executed request. This ID is not globally consistent and can
  therefore only be used to distinguish between different sources in a
  single subject.

- unsigned long sink: this contains the ID of the relay point that received
  the currently executed request.

Reserved subject commands (can ONLY be called within a subject):
(All functions with int return value 0 if not successful and 1 if successful.)

- Subject *new(<UserSubject>, <UserObject> *o | NONE): this generates a new
  subject of type UserSubject and returns a Subject pointer to it. Do NOT use
  'new UserSubject' to generate a new subject because this subject will NOT
  work properly in the Subjects environment.

- Subject *wakeupCocoon(c): this activates the cocoon c and returns a Subject
  pointer to it. A cocoon can only be used ONCE to create a subject.

- delete(s), s is of type UserSubject: this deletes s and all subjects spawned
  by s. Do NOT use 'delete s' because this will NOT work properly in the
  Subjects environment. s must have been created by the subject calling
  'delete(s)' for the operation to work.

- new Relay: this creates a new relay point with a connection to the subject
  creating it.

- new Relay(Identity *i): this creates a new relay point r with a connection
  to the relay point r' of identity i (if i is public or was meant for that
  subject). Identity i is destroyed after the call and set to NULL.

- delete r: this destructs relay point r. r must be owned by the subject for
  the operation to work.

- int call(verb, object | NONE): generates a call for action verb(object)
  within the given subject.

- int wakeup(Subject *s): if the subject calling wakeup(s) also generated s,
  s will be woken up. (A subject is awake by default.)

- int freeze(Subject *s): if the subject calling freeze(s) also generated s,
  s will be frozen. (That is, no more operation will be processed in s and
  its descrendents until wakeup(s) is called.)

- int wakeup(Relay *r): if the subject calling wakeup(r) also generated r,
  r will be woken up. (A relay point is awake by default.)

- int freeze(Relay *r): if the subject calling freeze(r) also generated r,
  r will be frozen. (That is, no more operation will be processed by r until
  wakeup(r) is called.)

- int idle(): is 0 if the subject still has at least one message to process.
  Otherwise, it is 1.

- int idle(Relay *r): is 0 if the local relay point r still has at least
  one message to process. Otherwise, it is 1.

- int indeg(Relay *r): returns number of incoming links into r

- int outdeg(Relay *r): returns number of outgoing links of r (0 or 1)

- Identity *extractIdentity(Relay *r): returns Identity that created Relay r
  and kills r

Reserved Relay variables and commands (can ONLY be accessed by subject owning
relay point):

- insigned long ID: identification number of the relay point

- int call(verb, object | NONE): creates a request in the relay to call
  verb(object) (relay point MUST be local to subject executing "call"). The
  call will be forwarded along outgoing connections of relay points until it
  reaches the base subject of the relay path, where it will be executed.


Remember:
- do NOT use nested "new" operations!
  (that is, operations like "new(subject(new object))" should be avoided)
- create/delete subjects via new(...) and delete(...)
- create/delete relay points and objects via new and delete without '()'
- an identity can only be used ONCE to create a relay point and ONLY by the
  subject for which it was created, if it is private
- a cocoon can only be used ONCE to create a new subject
- once a request verb(object) has been executed by some subject A, A gives up
  the ownership of that object, i.e., the object will be NULL

Background information and some tips:
- Subjects and objects do not have identities, only relay points do, to
  prevent denial-of-service attacks and identity theft and limit exposure as
  much as this is possible.

- Objects must be passive because if they were active, they can potentially
  freeze a subject. In general, the correctness and termination
  properties of a subject become hard to verify in a dynamic environment if
  methods beyond the given subject methods are callable in that subject.
  Executable objects can only be transmitted in form of cocoons, which will
  spawn new subjects.
  A subject owning an object has full access rights for any application-level
  information in it. If the access rights to an object should be restricted
  (due to copyright reasons), then the cocoon concept should be used to
  encapsulate an object within a subject enforcing these access rights before
  passing it on to another subject.

- Dark objects (identities and cocoons) cannot be copied to avoid
  denial-of-service attacks.

- The way the subjects environment is set up, relay points can never form
  cycles but they can, in principle, form arbitrary rooted trees. Also,
  packets cannot be duplicated in the relay point layer. This makes sure that
  the relay layer is safe against denial-of-service attacks that happen
  unnoticed by the subjects.

*/

#include <iostream>

// WARNING messages to user: 0:OFF, 1:ON

#define WARNING 1

// DEBUG messages to user: 0:OFF, 1:ON, 2:step-by-step
// Relays (including subject relays) and messages are numbered consecutively,
// starting with 0.

#define DEBUG 0

// maximum number of relay points; if beyond, then warning

#define MAXRELAYS 40000

// maximum number of subjects; if beyond, then warning

#define MAXSUBJECTS 10000

// maximum number of objects; if beyond, then warning

#define MAXOBJECTS 40000

// maximum number of messages; if beyond, then warning

#define MAXMESSAGES 40000


// =======================================================================
//  DO NOT CHANGE ANYTHING BELOW THIS LINE
// =======================================================================

// forward definitions of reserved classes

class Relay;      // class for relay points
class Subject;    // base class for subjects
class Object;     // base class for objects
class Identity;   // class for identities
class Cocoon;     // class for cocoons
class _Message;   // message storing a call verb(object)
class _Queue;     // FIFO queue of messages
class _Link;      // contains link to relay point
class _rLinks;    // list of relay pointers
class _sLink;     // contains link to subject
class _sLinks;    // list of subject pointers
class _idLinks;   // list of identities

// =======================================================================
//  Definitions
// =======================================================================

// define new types

#define SubjectType(type) \
    class type: public Subject \
 
#define FirstAction(type,verb) \
    type(Object* &object) { _handle((Subject *) this, new _Message((_SubjectCall) &type::verb, object)); }

#define ObjectType(type) \
    class type: public Object

#define RelayType(type) \
    class type: public Relay

#define Action void

// start Subjects environment

#define runSubjects(subject,object,time) \
    _runSubjects(new subject((Object* &) object), time)

// generates new subject

#define new(subject,object) \
    _create((Subject *) this, (Subject *) new subject((Object* &) object))

// generates new (public or private) cocoon

#define publicCocoon(subject,object) \
    _cocoon((Subject *) this, (Subject *) new subject((Object* &) object))

#define privateCocoon(subject, object, relay) \
    _cocoon((Subject *) this, (Subject *) new subject((Object* &) object), relay)

#define wakeupCocoon(cocoon) \
    _create((Subject *) this, (Subject *) new Subject((Cocoon* &) cocoon))

// deletes a subject

#define delete(subject) \
    _kill((Subject *) this, subject)

// wakes up a subject

#define wakeup(subject) \
    _wake((Subject *) this, subject)

// freezes a subject or relay

#define freeze(subject) \
    _halt((Subject *) this, subject)

// extracts identity and destroys relay

#define extractIdentity(relay) \
    _extract((Subject *) this, relay)

// idle checks whether a subject still has messages

// #define idle(subject)
//    _empty((Subject *) this, subject)

// call creates a new request to call verb(object)

#define call(verb, object) \
    _handle((Subject *) this, new _Message((_SubjectCall) &verb, (Object* &) object))

// _SubjectCall points to a Subject function

typedef void (Subject::*_SubjectCall)(Object *);

// for debugging IDs

typedef unsigned long ulong;


// =======================================================================
//   Object class
// =======================================================================

// Object counter for debugging

extern ulong _numObjects;

// Object class

class Object
{
public:
    ulong _debugID;
    Subject *_owner;  // current owner
    Relay *_source;   // source
    Relay *_sink;     // destination (NULL if public)

    // constructor of public object
    inline Object();

    // constructor of private object for r
    // Object(Relay *r);
};


// =======================================================================
//   Identity class
// =======================================================================

class Identity: public Object
{
public:
    Relay *_base;    // base of identity (endpoint of relay path)

    // constructor of empty identity (cannot be used)
    inline Identity();

    // constructor of public identity of r
    inline Identity(Relay *r);

    // constructor of private identity of r for r'
    inline Identity(Relay *r1, Relay *r2);

    // copying of identity not allowed
    inline Identity(const Identity &d);

    // destructor
    inline ~Identity();
};


// =======================================================================
//   Cocoon class
// =======================================================================

class Cocoon: public Object
{
public:
    Subject *_cocoon;   // cocoon

    // constructor of public cocoon
    inline Cocoon(Subject *home, Subject *s);

    // constructor of private cocoon for r
    inline Cocoon(Subject *home, Subject *s, Relay *r);

    // copying of cocoon not allowed
    inline Cocoon(const Cocoon &c);

    // destructor; if cocoon not used, then destructed
    inline ~Cocoon();
};


// =======================================================================
//   Relay class
// =======================================================================

// Relay point counter for debugging

extern ulong _numRelays;

// special types of relay points

#define SINK 0
#define ROOT 1

// class

class Relay
{
public:
    ulong ID;                  // identification number
    int _awake;                // 0: frozen, 1: awake
    int _private;              // 0: public, 1: private

    Subject *_home;            // subject associated with relay point
    _rLinks *_iLinks;          // incoming connections
    Relay  *_oLink;            // outgoing connection
    Relay  *_base;             // base of outgoing connection
    _Queue *_oQueue;           // message queue
    _idLinks *_identities;     // list of identities of relay


    // create new relay point
    inline Relay();

    // copy constructor
    inline Relay(const Relay &r);

    // constructor for parent and sink
    inline Relay(int type, Subject *s);

    // create new relay point with connection to relay identified by d
    inline Relay(Identity* &d);

    // set up relay point
    inline virtual void _setup();

    // s asks relay to handle call request
    inline virtual int _handle(Subject *s, _Message *m);

    // processes a message
    inline virtual int _process();

    // self-destructor
    inline virtual void _destruct();

    // default destructor
    inline ~Relay();
};


// =======================================================================
//   Subject class
// =======================================================================

// Subject counter for debugging

extern ulong _numSubjects;

// stores currently active subject

extern Subject *_activeSubject;

// class

class Subject
{
public:
    ulong _debugID;            // ID of subject
    Subject *_parent;          // parent subject of subject
    Subject *_child;           // child (only set if relay subject)
    Relay *_sink;              // relay point for messages to subject
    Relay *parent;             // relay point for messages to parent

    _rLinks *_Relays;          // relay points created by subject
    _rLinks *_activeRelays;    // active relay points
    _sLinks *_Subjects;        // subjects created by subject
    _sLinks *_activeSubjects;  // active subjects


    // base constructor
    inline Subject();

    // copy constructor
    inline Subject(const Subject &s);

    // creates a subject from a cocoon
    inline Subject(Cocoon* &d);

    // creates a subject representing s in its parent
    inline Subject(Subject *s);

    // set up subject
    inline virtual void _setup();

    // processes a subject
    inline virtual int _process();

    // self-destructor
    inline virtual void _destruct();

    // default destructor
    inline ~Subject();

protected:

    ulong source;        // source of current request
    ulong sink;          // relay point at which current request arrived

    // creates new subject
    inline Subject *_create(Subject *s, Subject *t);

    // create cocoon
    inline Cocoon *_cocoon(Subject *home, Subject *s);

    // create cocoon to r
    inline Cocoon *_cocoon(Subject *home, Subject *s, Relay *r);

    // deletes subject
    inline virtual int _kill(Subject *s, Subject *t);

    // deletes relay point
    inline virtual int _kill(Subject *s, Relay *r);

    // deletes relay point and returns identity
    inline Identity *_extract(Subject *s, Relay* &r);

    // s wakes up subject
    inline virtual int _wake(Subject *s, Subject *t);

    // s wakes up relay point
    inline virtual int _wake(Subject *s, Relay *r);

    // s freezes subject
    inline virtual int _halt(Subject *s, Subject *t);

    // s freezes relay point
    inline virtual int _halt(Subject *s, Relay *r);

    // s sends itself call request
    inline virtual int _handle(Subject *s, _Message *m);

    // subject checks if it still has requests in its buffer
    inline virtual int idle();

    // subject checks whether r still has requests in its butter
    inline virtual int idle(Relay *r);

    // subject returns number of incoming links of r
    inline virtual int indeg(Relay *r);

    // subject returns number of outgoing links of r (0 or 1)
    inline virtual int outdeg(Relay *r);

};


// =======================================================================
//   messages, queues and links
// =======================================================================

// Message counter for debugging

extern ulong _Messages;

// _Message provides simple container for messages

class _Message
{
public:
    ulong debugID;         // ID of message for debugging
    ulong source;          // source ID of message
    ulong sink;            // sink ID of message
    _SubjectCall VerbPtr;  // verb
    Object *ObjPtr;      // object
    _Message *next;        // pointer to next message in list

    // constructor
    inline _Message(_SubjectCall c, Object* &p) {
        debugID = _Messages;
        _Messages++;
        source = 0;
        sink = 0;
        if (_Messages > MAXMESSAGES && WARNING) {
            std::cout << "-- Warning! More than " << MAXMESSAGES << " messages.\n";
        }
        VerbPtr = c;
        ObjPtr = p;
        next = NULL;
        p = NULL;  // Object now gone for calling subject
    }
};


// _Queue provides a FIFO queue of messages

class _Queue
{
    _Message *first, *last;  // first, last element of FIFO queue

public:
    inline _Queue() {
        first = last = NULL;
    }

    // enqueue message
    inline void enqueue(_Message *m) {
        if (m == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! NULL message cannot be added to queue.\n";
            }
        } else {
            if (last == NULL) { // contains no message
                first = last = m;
            } else {         // contains at least one message
                last->next = m;
                last = m;
            }
            m->next = NULL;
        }
    }

    // dequeue message
    inline _Message *dequeue() {
        _Message *m;

        if (first == NULL) { // contains no message
            m = NULL;
        } else {          // contains at least one message
            m = first;
            first = first->next;
            if (first == NULL) {
                last = NULL;    // no further message
            }
            m->next = NULL;
        }
        return m;
    }

    // checks if queue is empty
    inline int empty() {
        return (first == NULL);
    }

    // delete queue
    inline ~_Queue() {
        _Message *m;

        while (first != NULL) {
            m = first;
            first = first->next;
            delete m;
        }
        last = NULL;
    }
};


// _Link provides simple container for relay points

class _Link
{
public:
    Relay *relay;        // pointer to relay point
    _Link *next;         // pointer to next link in list

    inline _Link(Relay *r) {
        relay = r;
        next = NULL;
    }
};


// _rLinks provides list of _Link elements

class _rLinks
{
    _Link *first, *last;  // first, last element of list

public:
    inline _rLinks() {
        first = last = NULL;
    }

    inline void add(Relay *r) {
        _Link *l;
        int contained;

        if (r == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cannot add NULL to list of relay points.\n";
            }
        } else {
            contained = 0;      // r already in list?
            l = first;
            while (l != NULL) {
                if (l->relay == r) {
                    contained = 1;
                }
                l = l->next;
            }
            if (!contained) {   // no: add r to end of list
                l = new _Link(r);
                if (last == NULL) { // list empty
                    first = last = l;
                } else {          // list not empty
                    last->next = l;
                    last = l;
                }
            }
        }
    }

    inline void remove(Relay *r) {
        _Link *l, *tl;

        if (first != NULL) { // list non-empty
            l = first;
            while (l != NULL && l->relay == r) {
                tl = l;
                l = l->next;
                delete tl;                 // remove all r's in front of list
            }
            first = l;                   // l==NULL or l->relay!=r
            if (l == NULL) {
                last = l;    // l==NULL: done
            } else {                     // l->relay!=r: continue with l->next
                while (l->next != NULL) {
                    if (l->next->relay == r) {
                        tl = l->next;
                        l->next = l->next->next;
                        delete tl;
                    } else {
                        l = l->next;
                    }
                }
                last = l;                  // l points to (new) last element
            }
        }
    }


    inline void wake(Relay *r) {
        _Link *l;

        l = first;
        while (l != NULL) {
            l->relay->_oLink = r;    // activate links to relay
            l = l->next;
        }
    }


    inline void halt() {
        _Link *l;

        l = first;
        while (l != NULL) {
            l->relay->_oLink = NULL; // deactivate links to relay
            l = l->next;
        }
    }


    inline int process() {
        _Link *l;
        int busy;

        l = first;
        busy = 0;
        while (l != NULL) {
            busy = busy | l->relay->_process();
            l = l->next;
        }
        return busy;
    }


    inline int empty() {
        return (first == NULL);
    }


    inline int idle() {
        _Link *r;
        int allidle;

        allidle = 1;
        r = first;
        while (r != NULL && allidle) {
            if (r->relay != NULL) {
                allidle = r->relay->_oQueue->empty();
            }
            r = r->next;
        }
        return allidle;
    }


    inline int count() {
        _Link *r;
        int i;

        r = first;
        i = 0;
        while (r != NULL) {
            i++;
            r = r->next;
        }
        return i;
    }


    inline void deleteRelays() {
        _Link *l;

        l = first;
        while (l != NULL) {
            l->relay->_destruct();      // remove all relay points
            l = l->next;
        }
    }


    inline ~_rLinks() {
        _Link *l;

        l = first;
        while (first != NULL) {
            first = first->next;
            delete l;
            l = first;
        }
    }
};


// _sLink provides simple container for subjects

class _sLink
{
public:
    Subject *subject;    // pointer to subject
    _sLink *next;        // pointer to next link in list

    inline _sLink(Subject *s) {
        subject = s;
        next = NULL;
    }
};


// _sLinks provides simple list of _sLink elements

class _sLinks
{
    _sLink *first, *last;  // first, last element of list

public:
    inline _sLinks() {
        first = last = NULL;
    }

    inline void add(Subject *s) {
        _sLink *l;
        int contained;

        if (s == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cannot add NULL to list of subjects.\n";
            }
        } else {
            contained = 0;      // r already in list?
            l = first;
            while (l != NULL) {
                if (l->subject == s) {
                    contained = 1;
                }
                l = l->next;
            }
            if (!contained) {   // no: add r to end of list
                l = new _sLink(s);
                if (last == NULL) { // list empty
                    first = last = l;
                } else {          // list not empty
                    last->next = l;
                    last = l;
                }
            }
        }
    }

    inline void remove(Subject *s) {
        _sLink *l, *tl;

        if (first != NULL) { // list non-empty
            l = first;
            while (l != NULL && l->subject == s) {
                tl = l;
                l = l->next;
                delete tl;                 // remove all s's in front of list
            }
            first = l;                   // l==NULL or l->subject!=s
            if (l == NULL) {
                last = l;    // l==NULL: done
            } else {                     // l->subject!=s: continue with l->next
                while (l->next != NULL) {
                    if (l->next->subject == s) {
                        tl = l->next;
                        l->next = l->next->next;
                        delete tl;
                    } else {
                        l = l->next;
                    }
                }
                last = l;                  // l points to (new) last element
            }
        }
    }


    inline int process() {
        _sLink *l;
        int busy;

        l = first;
        busy = 0;
        while (l != NULL) {
            busy = busy | l->subject->_process();
            l = l->next;
        }
        return busy;
    }


    inline int empty() {
        return (first == NULL);
    }


    inline void deleteSubjects() {
        _sLink *l;

        l = first;
        while (l != NULL) {
            l->subject->_destruct();      // remove all subjects
            l = l->next;
        }
    }


    inline ~_sLinks() {
        _sLink *l;

        l = first;
        while (first != NULL) {
            first = first->next;
            delete l;
            l = first;
        }
    }
};


// _idLinks allows identities to organize in a list

class _idLinks
{
public:
    _idLinks *prev;  // previous identity in id list
    _idLinks *next;  // next identity in id list
    Identity *id;

    inline _idLinks(Identity *ident) {
        prev = NULL;
        next = NULL;
        id = ident;
    }
};



// =======================================================================
//   Object functions
// =======================================================================

// constructor of pubic object
Object::Object()
{
    _debugID = _numObjects;
    _numObjects++;
    if (WARNING && _numObjects > MAXOBJECTS) {
        std::cout << "-- Warning! More than " << MAXOBJECTS << " objects.\n";
    }
    _owner = NULL;
    _source = NULL;
    _sink = NULL;
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "--Warning! Object created outside of subjects environment.\n";
        }
    } else {
        _owner = _activeSubject;
        if (DEBUG) {
            std::cout << "-- Creating object " << _debugID << " by subject " << _activeSubject->_debugID << ".\n";
        }
    }
}

/*
  // constructor of private object for r
  Object::Object(Relay *r)
  {
    _debugID = _numObjects;
    _numObjects++;
    if (WARNING && _numObjects>MAXOBJECTS)
      std::cout << "-- Warning! More than " << MAXOBJECTS << " objects.\n";
    _owner = NULL;
    _source = NULL;
    _sink = NULL;
    if (_activeSubject == NULL) {
      if (WARNING)
        std::cout << "--Warning! Object created outside of the subjects environment.\n";
    }
    else {
      _owner = _activeSubject->_sink;
      if (r==NULL) {
        if (WARNING)
          std::cout << "--Warning! Destination of object not well-defined.\n";
      }
      else {
        if (r->_home!=_activeSubject || r->_base==NULL) {
          if (WARNING)
            std::cout << "--Warning! Object cannot be created for " << r->ID << "\n.";
        }
        else {
          _sink = r->_base;  // indicates private object
          if (DEBUG) {
            if (_activeSubject == NULL) {
              if (WARNING)
                std::cout << "-- Warning! Object " << _debugID << " created outside of subjects environment.\n";
            }
            else
              std::cout << "-- Creating object " << _debugID << " by " << _activeSubject->_debugID << " for " << _sink->ID << ".\n";
          }
        }
      }
    }
  }
*/

extern Object  *NONE;  // empty object
// Object** _Temp = NULL; // temporary object for transfers

// =======================================================================
//   Identity functions
// =======================================================================

// constructor of empty identity
Identity::Identity()
{
    _source = NULL;
    _sink = NULL;
    _base = NULL;
    //if (WARNING)
    //  std::cout << "-- Warning! Creating empty identity.\n";
}


// constructor of public identity of r
Identity::Identity(Relay *r)
{
    _idLinks *idl;

    _source = NULL;
    _sink = NULL;
    _base = NULL;
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Identity not created within active subject.\n";
        }
    } else {
        if (r == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cannot create identity of NULL.\n";
            }
        } else {
            if (r->_home != _activeSubject) {
                if (WARNING) {
                    std::cout << "Warning! Subject " << _activeSubject->_debugID << " cannot create identity for relay " << r->ID << ".\n";
                }
            } else {
                if (DEBUG) {
                    std::cout << "-- Creating public identity of " << r->ID << ".\n";
                }
                _source = r;               // set source and base
                _base = r->_base;
                idl = new _idLinks(this);  // add identity at beginning of list
                if (r->_identities->next != NULL) {
                    r->_identities->next->prev = idl;
                }
                idl->next = r->_identities->next;
                idl->prev = r->_identities;
                r->_identities->next = idl;
            }
        }
    }
}


// constructor of private identity of r1 for r2
Identity::Identity(Relay *r1, Relay *r2)
{
    _idLinks *idl;

    _source = NULL;
    _sink = NULL;
    _base = NULL;
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Identity not created within active subject.\n";
        }
    } else {
        if (r1 == NULL || r2 == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cannot create identity of or to NULL.\n";
            }
        } else {
            if (r1->_home != _activeSubject) {
                if (WARNING) {
                    std::cout << "Warning! Subject " << _activeSubject->_debugID << " cannot create identity of relay " << r1->ID << ".\n";
                }
            } else {
                if (r2->_home != _activeSubject) {
                    if (WARNING) {
                        std::cout << "Warning! Subject " << _activeSubject->_debugID << " cannot create identity for relay " << r2->ID << ".\n";
                    }
                } else {
                    if (DEBUG) {
                        std::cout << "-- Creating private identity of relay " << r1->ID << " for relay " << r2->ID << ".\n";
                    }
                    _source = r1;              // set source and base
                    _base = r1->_base;
                    idl = new _idLinks(this);  // add identity at beginning of list
                    if (r1->_identities->next != NULL) {
                        r1->_identities->next->prev = idl;
                    }
                    idl->next = r1->_identities->next;
                    idl->prev = r1->_identities;
                    r1->_identities->next = idl;
                    if (r2 != r2->_home->parent) {
                        _sink = r2->_base;
                    } else {
                        if (r2->_base != NULL) {
                            _sink = r2->_base->_oLink;
                        }
                    }
                }
            }
        }
    }
}


// copying of identity not allowed
Identity::Identity(const Identity &d)
{
    _source = NULL;
    _sink = NULL;
    _base = NULL;
    if (WARNING) {
        std::cout << "-- Warning! Identity cannot be copied.\n";
    }
}


// destructor
Identity::~Identity()
{
    _idLinks *idl;
    _idLinks *delidl;

    if (_source != NULL) {
        idl = _source->_identities;  // remove identity from id list
        while (idl->next != NULL && idl->next->id != this) {
            idl = idl->next;
        }
        if (idl->next != NULL) {
            delidl = idl->next;
            if (idl->next->next != NULL) {
                idl->next->next->prev = idl;
            }
            idl->next = idl->next->next;
            delete delidl;
        } else {
            std::cout << "Error: Identity missing in list!\n";
        }
    }
}


// =======================================================================
//   Cocoon functions
// =======================================================================

// constructor of public cocoon
Cocoon::Cocoon(Subject *home, Subject *s)
{
    _source = NULL;
    _sink = NULL;
    _cocoon = NULL;
    if (home != NULL && s != NULL) {
        if (DEBUG) {
            std::cout << "-- Creating cocoon of subject " << s->_debugID << ".\n";
        }
        _source = home->_sink;
        _cocoon = s;
    }
}

// constructor of cocoon for r
Cocoon::Cocoon(Subject *home, Subject *s, Relay *r)
{
    _source = NULL;
    _sink = NULL;
    _cocoon = NULL;
    if (home != NULL && s != NULL && r != NULL && r->_base != NULL) {
        if (DEBUG) {
            std::cout << "-- Creating cocoon of subject " << s->_debugID << " for " << r->_base->ID << ".\n";
        }
        _source = home->_sink;    // create cocoon
        if (r != r->_home->parent) {        // r not relay to parent?
            _sink = r->_base;
        } else {
            _sink = r->_base->_oLink;
        }
        _cocoon = s;
    }
}

// copying of cocoon not allowed
Cocoon::Cocoon(const Cocoon &c)
{
    _source = NULL;
    _sink = NULL;
    _cocoon = NULL;
    if (_activeSubject == NULL && WARNING) {
        std::cout << "-- Warning! Cocoon not created within active subject.\n";
    }
}


// destructor; if cocoon not used, then destructed
Cocoon::~Cocoon()
{
    if (_cocoon != NULL) {
        _cocoon->_destruct();
    }
}

extern Cocoon *_Cocoon;   // temporary cocoon for transfer

// =======================================================================
//   Relay functions
// =======================================================================

// create new relay point
Relay::Relay()
{
    _home = _activeSubject;
    _setup();
    if (_home != NULL) {
        if (DEBUG) {
            std::cout << "-- Creating relay point " << ID << " by subject " << _home->_debugID << ".\n";
        }
        if (_home->_sink != NULL) {
            _oLink = _home->_sink;     // Relay has link to own subject
            _oLink->_iLinks->add(this);
        }
    }
}


// copy constructor (like standard constructor)
Relay::Relay(const Relay &r)
{
    _home = _activeSubject;
    _setup();
    if (_home != NULL) {
        if (DEBUG) {
            std::cout << "-- Creating relay point " << ID << " by subject " << _home->_debugID << ".\n";
        }
        if (_home->_sink != NULL) {
            _oLink = _home->_sink;     // Relay has link to own subject
            _oLink->_iLinks->add(this);
        }
    }
}


// create sink or parent relay
Relay::Relay(int type, Subject *s)
{
    _home = s;
    _setup();
    if (_home != NULL) {
        if (type == ROOT) {
            if (_home->_parent != NULL && _home->_parent->_sink != NULL) {
                _oLink = _home->_parent->_sink;
                _oLink->_iLinks->add(this);
                if (DEBUG) {
                    std::cout << "-- Creating parent " << ID << " for subject " << _home->_debugID << ".\n";
                }
            }
        } else {
            if (DEBUG) {
                std::cout << "-- Creating sink " << ID << " for subject " << _home->_debugID << ".\n";
            }
        }
    }
}


// create new relay point with connection to relay identified by d
Relay::Relay(Identity* &d)
{
    _home = _activeSubject;
    _setup();
    if (_home != NULL) {    // relay generated within subject?
        if (d == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cannot establish connection from " << ID << " to NULL.\n";
            }
        } else {
            if (d->_source == NULL) {
                if (WARNING) {
                    std::cout << "-- Warning! Cannot establish connection from " << ID << " to NULL.\n";
                }
            } else {
                if (d->_base == NULL) {
                    if (WARNING) {
                        std::cout << "-- Warning! Private identity not correctly defined or already used.\n";
                    }
                } else {
                    if (d->_sink != NULL && d->_sink->_home != _home) {
                        if (WARNING) {
                            std::cout << "-- Warning! Identity of " << d->_source->ID << " not meant for " << ID << ".\n";
                        }
                    } else {
                        _oLink = d->_source;        // connect to relay point
                        _oLink->_iLinks->add(this);
                        if (d->_sink == NULL) {
                            _private = 0;
                        }
                        _base = d->_base;        // base = d-base
                        delete d;                // destroy identity
                        d = NULL;
                        if (DEBUG) {
                            std::cout << "-- Creating relay point " << ID << " to " << _oLink->ID << " with base " << _base->ID << ".\n";
                        }
                        if (!(_oLink->_awake)) {
                            _oLink = NULL;
                        }
                    }
                }
            }
        }
    }
}


// sets up relay point in subject creating it
void Relay::_setup()
{
    ID = _numRelays;
    _numRelays++;
    if (_numRelays > MAXRELAYS && WARNING) {
        std::cout << "-- Warning! More than " << MAXRELAYS << " relay points.\n";
    }

    _awake = 1;
    _private = 1;
    _base = this;
    _oLink = NULL;
    _oQueue = new _Queue();
    _iLinks = new _rLinks();
    _identities = new _idLinks(NULL);  // dummy element for _identities

    if (_home == NULL) {
        if (WARNING && ID > 0) {
            std::cout << "-- Warning! Relay point not created within active subject.\n";
        }
    } else {
        if (_home->_Relays != NULL) {
            _home->_Relays->add(this);
            _home->_activeRelays->add(this);
        }
    }
}


// handles call request
int Relay::_handle(Subject *s, _Message *m)
{
    if (s != _home) {
        if (WARNING) {
            std::cout << "-- Warning! Subject " << s->_debugID << " cannot call " << ID << "->call().\n";
        }
        delete m;
    } else {
        if (m != NULL) {  // message well-defined?
            if (DEBUG) {
                std::cout << "-- Message " << m->debugID << " stored in relay " << ID << ".\n";
            }
            m->source = ID;
            if (m->ObjPtr != NULL) {
                m->ObjPtr->_owner = NULL;    // object in transit
            }
            _oQueue->enqueue(m);
        }
        return 1;
    }
    return 0;
}


// forwards a message (only non-sink relay points)
int Relay::_process()
{
    _Message *m;

    if (_oLink != NULL) { // relay has outgoing link?
        if (_oLink == _home->_sink) {  // outgoing link to home?
            while (!_oQueue->empty()) {  // queue has messages?
                m = _oQueue->dequeue();
                m->sink = ID;      // set sink of message to current relay
                _oLink->_oQueue->enqueue(m);
            }
            if (DEBUG) {
                std::cout << "-- All messages in " << ID << " to " << _home->_sink->ID << ".\n";
            }
        } else {
            m = _oQueue->dequeue();
            if (m != NULL) {   // queue has message?
                _oLink->_oQueue->enqueue(m); // forward message along outgoing link
                if (DEBUG) {
                    std::cout << "-- Message " << m->debugID << "  => " << _oLink->ID << ".\n";
                }
                return 1;        // relay busy
            }
        }
    }
    return 0;
}


// self-destructor
void Relay::_destruct()
{
    delete this;
}


// destructor
Relay::~Relay()
{
    _idLinks *idl;

    if (DEBUG) {
        std::cout << "-- Relay " << ID << " deleted.\n";
    }
    if (_oLink != NULL) {
        _oLink->_iLinks->remove(this);    // cut outgoing link
    }
    delete _oQueue;                  // delete remaining messages
    if (_iLinks != NULL) {
        _iLinks->halt();               // cut all incoming links
        delete _iLinks;
    }
    if (_home != NULL && _home->_Relays != NULL) {
        _home->_Relays->remove(this);
        _home->_activeRelays->remove(this);
    }
    while (_identities != NULL) {
        idl = _identities;
        if (_identities->id != NULL) {
            _identities->id->_source = NULL;
        }
        _identities = _identities->next;
        delete idl;
    }
}


// =======================================================================
//   Subject functions
// =======================================================================

// default constructor
Subject::Subject()
{
    _setup();
    _Relays = new _rLinks();
    _Subjects = new _sLinks();
    _activeRelays = new _rLinks();
    _activeSubjects = new _sLinks();
    _sink = new Relay(SINK, (Subject *) this);

    if (_activeSubject == NULL && _debugID > 0) {
        if (WARNING) {
            std::cout << "-- Warning! Subject created outside of the subjects environment.\n";
        }
    } else {
        if (DEBUG) {
            std::cout << "-- Creating subject " << _debugID << ".\n";
        }
        _activeSubject = NULL;  // new subject can only use 'call' in constructor
    }
}


// copy constructor (for cloning)
Subject::Subject(const Subject &s)
{
    _setup();
    _Relays = new _rLinks();
    _Subjects = new _sLinks();
    _activeRelays = new _rLinks();
    _activeSubjects = new _sLinks();
    _sink = new Relay(SINK, (Subject *) this);

    if (_activeSubject == NULL && _debugID > 0) {
        if (WARNING) {
            std::cout << "-- Warning! Subject created outside of the subjects environment.\n";
        }
    } else {
        if (DEBUG) {
            std::cout << "-- Creating subject " << _debugID << ".\n";
        }
        _activeSubject = NULL;  // new subject can only use 'call' in constructor
    }
}


// creates a relay subject
Subject::Subject(Cocoon* &d)
{
    _setup();
    if (_activeSubject == NULL) {
        if (WARNING && _debugID > 0) {
            std::cout << "-- Warning! Subject created outside of the subjects environment.\n";
        }
    } else {
        if (DEBUG) {
            std::cout << "-- Creating subject " << _debugID << ".\n";
        }
        if (d == NULL || d->_cocoon == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cocoon not well-defined.\n";
            }
        } else {
            _parent->_Subjects->add(this);
            _parent->_activeSubjects->add(this);
            if (d->_sink != NULL && d->_sink->_home != _parent) {
                if (WARNING) {
                    std::cout << "-- Warning! Cocoon from " << d->_cocoon->_debugID << " not meant for " << _parent->_debugID << ".\n";
                }
            } else {
                if (DEBUG) {
                    std::cout << "-- Spawning cocoon " << d->_cocoon->_debugID << ".\n";
                }
                _child = d->_cocoon;
                d->_cocoon = NULL;
                delete d;                // destroy cocoon
                d = NULL;
                _child->_parent = _parent;
                _child->parent = new Relay(ROOT, _child);
            }
        }
    }
}


// creates a relay subject
Subject::Subject(Subject *s)
{
    _setup();
    if (_activeSubject == NULL) {
        if (WARNING && _debugID > 0) {
            std::cout << "-- Warning! Subject created outside of the subjects environment.\n";
        }
    } else {
        if (DEBUG) {
            std::cout << "-- Creating relay subject " << _debugID << ".\n";
        }
        if (s == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! New subject not well-defined.\n";
            }
        } else {
            _parent->_Subjects->add(this);
            _parent->_activeSubjects->add(this);
            if (DEBUG) {
                std::cout << "-- Activating subject " << s->_debugID << ".\n";
            }
            _child = s;
            _child->_parent = _parent;
            _child->parent = new Relay(ROOT, _child);
        }
    }
}


// set up subject
void Subject::_setup()
{
    _debugID = _numSubjects;
    _numSubjects++;
    if (_numSubjects > MAXSUBJECTS && WARNING) {
        std::cout << "-- Warning! More than " << MAXSUBJECTS << " subjects.\n";
    }
    _parent = _activeSubject;
    _child = NULL;

    _sink = NULL;
    parent = NULL;
    source = 0;
    sink = 0;

    _Relays = NULL;
    _Subjects = NULL;
    _activeRelays = NULL;
    _activeSubjects = NULL;
}


// creates a new cocoon
Cocoon *Subject::_cocoon(Subject *home, Subject *s)
{
    Cocoon *c;

    c = NULL;
    if (home == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Cocoon created outside of Subjects environment.\n";
        }
        if (s != NULL) {
            s->_destruct();
        }
    } else {
        if (s == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cocoon does not contain subject.\n";
            }
        } else {
            // if s is new then _activeSubject == NULL
            if (_activeSubject != NULL) {
                if (WARNING) {
                    std::cout << "-- Warning! Cocoon not created by newcocoon().\n";
                }
                s->_destruct();
            } else {
                _activeSubject = home;
                c = new Cocoon(home, s);
            }
        }
    }
    return c;
}


// creates a new cocoon for r
Cocoon *Subject::_cocoon(Subject *home, Subject *s, Relay *r)
{
    Cocoon *c;

    c = NULL;
    if (home == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Cocoon not created within active subject.\n";
        }
        if (s != NULL) {
            s->_destruct();
        }
    } else {
        if (s == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! Cocoon does not contain subject.\n";
            }
        } else {
            // if s is new then _activeSubject == NULL
            if (_activeSubject != NULL) {
                if (WARNING) {
                    std::cout << "-- Warning! Existing subject " << s->_debugID << " cannot be a cocoon.\n";
                }
                s->_destruct();
            } else {
                _activeSubject = home;
                if (r == NULL || r->_base == NULL) {
                    if (WARNING) {
                        std::cout << "-- Warning! Destination of cocoon not well-defined.\n";
                    }
                } else {
                    c = new Cocoon(home, s, r);
                }
            }
        }
    }
    return c;
}


// self-destructor
void Subject::_destruct()
{
    delete this;
}


// spawn new subject t in subject s
Subject *Subject::_create(Subject *s, Subject *t)
{
    Subject *rs;  // relay subject

    rs = NULL;
    if (s != this) {
        if (WARNING) {
            std::cout << "-- Warning! " << s->_debugID << " cannot call 'new' for " << _debugID << ".\n";
        }
        t->_destruct();
    } else {
        _activeSubject = s;  // recover _activeSubject (NULL after creation of t)
        rs = new Subject((Subject *) t);
    }
    return rs;
}


// kill relay point r in subject s
int Subject::_kill(Subject *s, Relay *r)
{
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'delete' outside of the subjects environment.\n";
        }
    } else {
        if (r == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot destruct NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'delete' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != r->_home) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'delete' for relay " << r->ID << ".\n";
                    }
                } else {
                    r->_destruct();  // delete relay r
                    return 1;
                }
            }
        }
    }
    return 0;
}


// kill relay point r in subject s and return identity
Identity *Subject::_extract(Subject *s, Relay* &r)
{
    Identity *id;
    _idLinks *idl;

    id = NULL;
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'delete' outside of the subjects environment.\n";
        }
    } else {
        if (r == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot destruct NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'delete' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != r->_home) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'delete' for relay " << r->ID << ".\n";
                    }
                } else {
                    id = new Identity;
                    id->_source = r->_oLink;
                    if (r->_private) {
                        id->_sink = _sink;
                    } else {
                        id->_sink = NULL;
                    }
                    id->_base = r->_base;
                    if (id->_source != NULL) {
                        idl = new _idLinks(id);  // add identity at beginning of list
                        if (id->_source->_identities->next != NULL) {
                            id->_source->_identities->next->prev = idl;
                        }
                        idl->next = id->_source->_identities->next;
                        idl->prev = id->_source->_identities;
                        id->_source->_identities->next = idl;
                    }
                    r->_destruct();  // delete relay r
                    r = NULL;
                }
            }
        }
    }
    return id;
}


// kill subject t in subject s
int Subject::_kill(Subject *s, Subject *t)
{
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'delete' outside of the subjects environment.\n";
        }
    } else {
        if (t == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot destruct NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'delete' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != t->_parent) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'delete' for subject " << t->_debugID << ".\n";
                    }
                } else {
                    t->_destruct();  // delete subject t
                    return 1;
                }
            }
        }
    }
    return 0;
}


// default destructor
Subject::~Subject()
{
    _rLinks *rl;
    _sLinks *sl;

    if (DEBUG) {
        std::cout << "-- Subject " << _debugID << " deleted.\n";
    }
    if (_child != NULL) { // relay subject?
        delete _child;
        if (_parent != NULL && _parent->_Subjects != NULL) {
            _parent->_Subjects->remove(this);
            _parent->_activeSubjects->remove(this);
        }
    } else { // bad relay subject or standard subject
        if (_Relays != NULL) {        // standard subject
            rl = _Relays;               // necessary to avoid self-removal of
            _Relays = NULL;             // relay points from _Relays
            rl->deleteRelays();         // delete relays (including _sink and parent)
            delete rl;                  // delete list of relays
            delete _activeRelays;
            sl = _Subjects;             // dito
            _Subjects = NULL;
            sl->deleteSubjects();       // delete subjects
            delete sl;                  // delete list of subjects
            delete _activeSubjects;
        }
    }
}


// handles call request
int Subject::_handle(Subject *s, _Message *m)
{
    // only here, _activeSubject==NULL is allowed
    if (m == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << " cannot handle NULL.\n";
        }
    } else {
        if (s != this) {
            if (WARNING) {
                std::cout << "-- Warning! " << s->_debugID << " cannot call 'call' for subject " << _debugID << ".\n";
            }
            delete m;
        } else {
            if (_sink != NULL) {
                if (DEBUG) {
                    std::cout << "-- Message " << m->debugID << " stored in subject " << _debugID << ".\n";
                }
                _sink->_oQueue->enqueue(m);
                return 1;
            } else {
                if (WARNING) {
                    std::cout << "-- Warning! Sink of subject " << _debugID << " not well-defined.\n";
                }
                delete m;
            }
        }
    }
    return 0;
}


// s wakes up relay point r
int Subject::_wake(Subject *s, Relay *r)
{
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'wakeup' outside of the subjects environment.\n";
        }
    } else {
        if (r == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot wake up NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'wakeup' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != r->_home) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'wakeup' for relay " << r->ID << ".\n";
                    }
                } else {
                    r->_awake = 1;
                    _activeRelays->add(r);  // wake up relay point r
                    r->_iLinks->wake(r);    // wake up connections to r
                    if (DEBUG) {
                        std::cout << "-- Subject " << _debugID << " wakes up relay " << r->ID << ".\n";
                    }
                    return 1;
                }
            }
        }
    }
    return 0;
}


// s wakes up subject t
int Subject::_wake(Subject *s, Subject *t)
{
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'wakeup' outside of the subjects environment.\n";
        }
    } else {
        if (t == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot wake up NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'wakeup' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != t->_parent) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'wakeup' for subject " << t->_debugID << ".\n";
                    }
                } else {
                    _activeSubjects->add(t);  // wake up subject t
                    return 1;
                }
            }
        }
    }
    return 0;
}


// s freezes relay point r
int Subject::_halt(Subject *s, Relay *r)
{
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'freeze' outside of the subjects environment.\n";
        }
    } else {
        if (r == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot freeze NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'freeze' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != r->_home) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'freeze' for relay " << r->ID << ".\n";
                    }
                } else {
                    r->_awake = 0;
                    _activeRelays->remove(r);  // freeze relay point r
                    r->_iLinks->halt();        // freeze connections to r
                    if (DEBUG) {
                        std::cout << "-- Subject " << _debugID << " freezes relay " << r->ID << ".\n";
                    }
                    return 1;
                }
            }
        }
    }
    return 0;
}


// s freezes subject t
int Subject::_halt(Subject *s, Subject *t)
{
    if (_activeSubject == NULL || (_parent == NULL && _debugID > 0)) {
        if (WARNING) {
            std::cout << "-- Warning! " << _debugID << "called 'freeze' outside of the subjects environment.\n";
        }
    } else {
        if (t == NULL) {
            if (WARNING) {
                std::cout << "-- Warning! " << _debugID << " cannot freeze NULL.\n";
            }
        } else {
            if (s != this) {
                if (WARNING) {
                    std::cout << "-- Warning! " << s->_debugID << " cannot call 'freeze' for subject " << _debugID << ".\n";
                }
            } else {
                if (s != t->_parent) {
                    if (WARNING) {
                        std::cout << "-- Warning! " << _debugID << " cannot call 'freeze' for subject " << t->_debugID << ".\n";
                    }
                } else {
                    _activeSubjects->remove(t);  // freezes subject t
                    return 1;
                }
            }
        }
    }
    return 0;
}


// processes a subject
int Subject::_process()
{
    _Message *m;
    int busy;

    busy = 0;
    if (_child != NULL) {  // relay subject
        busy = _child->_process();
    } else {               // bad relay subject or standard subject
        if (_activeSubjects != NULL) {            // standard subject
            busy = _activeSubjects->process();      // first, process children
            _activeSubject = this;
            busy = busy | _activeRelays->process(); // then active relays
            m = _sink->_oQueue->dequeue();          // then subject itself
            if (m != NULL) {
                if (DEBUG) {
                    std::cout << "-- Executing message " << m->debugID << " in subject " << _debugID << ".\n";
                }
                source = m->source + _debugID;        // create local ID of source
                sink = m->sink;                       // set sink to sink of message
                if (m->ObjPtr != NULL) {
                    m->ObjPtr->_owner = this;    // set new owner of object
                }
                if (m->ObjPtr == NULL || m->ObjPtr->_sink == NULL || m->ObjPtr->_sink->_home == this) {
                    (this->*(m->VerbPtr))(m->ObjPtr);    // execute message
                } else {
                    if (WARNING) {
                        std::cout << "-- Warning! Message " << m->debugID << " not meant for subject " << _debugID << ".\n";
                    }
                }
                delete m;
                busy = 1;
            }
        }
    }
    return busy;
}


// subject checks if it has no more messages in queues
int Subject::idle()
{
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Method 'idle' called outside of the subjects environment.\n";
        }
    } else {
        if (_activeSubject != this) {
            if (WARNING) {
                std::cout << "-- Warning! Subject " << _activeSubject->_debugID << " cannot call " << _debugID << "->idle().\n";
            }
        } else {
            return _Relays->idle();
        }
        // return _sink->_oQueue->empty();
    }
    return 0;
}


// checks if Relay r has no more messages
int Subject::idle(Relay *r)
{
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Method 'idle' called outside of the subjects environment.\n";
        }
    } else {
        if (r != NULL) {
            if (_activeSubject != r->_home) {
                if (WARNING) {
                    std::cout << "-- Warning! Subject " << _activeSubject->_debugID << " cannot call " << _debugID << "->idle().\n";
                }
            } else {
                return r->_oQueue->empty();
            }
        }
    }
    return 0;
}


// checks if Relay r has alive outgoing connection
int Subject::indeg(Relay *r)
{
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Method 'idle' called outside of the subjects environment.\n";
        }
    } else {
        if (r != NULL) {
            if (_activeSubject != r->_home) {
                if (WARNING) {
                    std::cout << "-- Warning! Subject " << _activeSubject->_debugID << " cannot call " << _debugID << "->idle().\n";
                }
            } else {
                return r->_iLinks->count();
            }
        }
    }
    return 0;
}


// returns number of incoming connections of Relay r
int Subject::outdeg(Relay *r)
{
    if (_activeSubject == NULL) {
        if (WARNING) {
            std::cout << "-- Warning! Method 'idle' called outside of the subjects environment.\n";
        }
    } else {
        if (r != NULL) {
            if (_activeSubject != r->_home) {
                if (WARNING) {
                    std::cout << "-- Warning! Subject " << _activeSubject->_debugID << " cannot call " << _debugID << "->idle().\n";
                }
            } else {
                if (r->_oLink == NULL) {
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }
    return 0;
}



// =======================================================================
//   Base of Subjects environment
// =======================================================================

inline void sys_pause()
{
    std::cout << "press [return] to continue \n";
    char c;
    do {
        std::cin.get(c);
    } while (std::cin.good() && c != '\n');
}


// runs the Subjects environment
inline void _runSubjects(Subject *s, unsigned long T)
{
    unsigned long t;
    int busy;

    if (T == 0) {
        T = 0xFFFFFFFF;
    }
    t = 1;
    busy = 1;
    while (t <= T && busy) {
        if (DEBUG > 1) {   // step-by-step mode
            sys_pause();
            std::cout << "\n";
            std::cout << "-- Round " << t << ":\n";
        }
        busy = s->_process();
        t++;
    }
}

#endif  // SUBJECTS1_6_H_
