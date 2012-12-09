#Definitions

**Entity**: A positive, unique number that pertains to a reference of something, which is constructed with properties and attributes.

**Property**: A textual name, which may also be rendered to a 32 bit or 64 bit integer, which identifies some categorical group. A mapping may exist from a property to a property contract.

**Property Contract**: A configuration of names (which may be rendered as a 32 bit or 64 bit integer) and enumeration entries defining the type for properties. A default is also specified.

**Attribute Value**: a typed value and the enumeration for the type.

**Attribute Wrapping**: The Attribute Value and the name which may be a string and or a 32 bit or 64 bit integer.

**Component**: Code associated to a non-empty set of properties, which acts on all entities which have a subset of properties equal to the components properties, using the attributes provided by the Property Contract.

**System**: A collection of related Components, subsystems, libraries, assistants and wrappers.

**Library**: A discrete collection of related code that computes and or interacts with the user.

**Assistant**: A collection of related routines which are optimized for some operation.

**Wrapper**: Acts as an abstraction to the rest of the system for an external library. 

**Actor**: An active client that listens and emits and can be individually identified.

#Design

The data server is a thread or process that interacts with the host game, and for local instances which have their own independent server. 

To remain within an achievable scope, this design will not take into account sharding, distribution, etc. All data replication and authentication is to be done by an external server. 

##Requirements

###Actors

Actors must be **retained** and **separately selectable**, the actor is to be _embedded in the Entity ID_.
This can be supplemented by having an additional entity, where the type is a key value pair of the actor ID and an array of entities.

###Data

+ A list of Entity IDs
+ A property in string and possibly numeric _(hash)_ form.
+ Attribute Value
+ Attribute Wrappers for Stringed names and integral names.
+ Property Contract Definition
+ Contract Field
+ Complete Entity, using String Names

###Events

**Server to Client** and **Client to Server**

+ Entity Attributes Changed
+ Entity Created
+ Entity Property Added
+ Entity Property Removed
+ Entity Changing ID

**Client to Server**

+ Entity Destroyed
+ Property Set is listened to, _server keeps a ref count_
+ Property Set is no longer listened to

###Transport

Ideally, Listen and operate on a single port.

###Server Side

Must be able to serialize selectively, and be able to unserialize and instantiate batches.
Any data that conflicts will be dropped and created from the batch, never merged for a batch.

Have Configurable 

+ Port_(s)_
+ Number of Worker instances, _when allowing external communication_

The server will offer interprocess and in-process channels for communication.
The names will be constant, for now only one server at a time is considered.

###Client Side

The client side needs an interface to fetch the following
Entities by ID in batch
All Entities containing a set of properties

The client side needs to be able to subscribe to the following

+ Entity Attribute Changed, by a set of Properties. 
+ Entity Created, by a set of Properties
+ Entity removed by a set of Properties
+ **Entity changing IDs**

Entity Destruction and entity removed are the same to a component on the client side. The client issues a command for destruction.

There is a client context, which handles communication with the server or service, including the transport option. The context will offer a means for specialized context instances, used for _components_.

##Data Structure

###Property

Holds contracts and references to sets which are contracted with it. 

The Contract holding a list of attribute templates being

+ Name
+ Default value
+ Type, _though the type is implicit in the default value_
+ Contracted Reference Sets,
	_An array of sets which include this property. 
	Used for updating when the contract definition is augmented._

###Property Set

Holds the information which reading queries hit as a pivot before grabbing the entities.

+ Set of properties that compose this set
+ A set of Entity IDs
+ Cached Property Contract, _same format as in the Property Data Structure_
+ Property Definition

###Entity

Holds the actual data contents that the user expects.

+ Entity ID
+ Set of Properties
+ Set of Attributes


#Plans of Implementation

##Support

###Templated Set class
Which supports

+ Union
+ Intersection
+ Difference
+ Symmetric Difference
+ Powerset

The first four operations are already in the standard library, however they are not built into the existing set class. 
They operate efficiently on sorted iterable objects, such as vector. 

Powerset needs to be efficient. Also, there needs to be an **iterable** powerset, without generating the entire powerset.

##Server
There is a singleton **Server Manager** which is manages configuration, start up, shut down. This manager is what the embedded application, or standalone executable uses.

The Server Manager controls the number of available workers, back-end engine, and the communication settings.
In-process communication will always be available, interprocess and network communication is something that must be explicitly enabled.

The ZeroMQ sockets can bind to multiple methods I believe, so the distinction between which is not known nor important to the server.

Most of the design has been purposefully made to be asynchronous, the events can be sent out through a publication-subscribe pattern. Listening to events will be similar, the clients push messages out, expecting that the context will deliver them and the server process them.

###Back-end
The back-end engine inherits an interface which is used by workers and embedded applications. A parameter at startup is given by name which selects the engine to use.

An initial naïve approach will be implemented using locks. **Poco Foundation** provides read-write locks which are adequate for this.

Eventually, a lock-free or minimal locking design would be desired. 

An _STL_ map will be used for the following

+ **Key: String**;
	Value: Property
+ **Key: String Set**;
	Value: Property Set
+ **Key: Integer**;
	Value: Entity
+ **Key: Integer Mask**
	Value: Actor

Furthermore, _Property Sets_ will contain a list of client identifiers
which will be used to broadcast to the clients the change.

These client identifiers are populated on client connection with what they register with.

####Relations

A **Property** holds a contract, and references to all property sets which contain it.

A **Property Set** holds a reference to all entities which have them.

An **Actor** holds references to entities which they own.

###Client Registration
Clients will connect using a PAIR socket, the server will always have one socket open not being used, once it is connected to, another thread is created which hosts another socket.

On registration, the database will give the client an identifier and a key, the two are sent back with each message. 
Once the client gets its updating credentials, it will then notify the server of all the contracts that it has.
The contracts are transactional, meaning that until it is determined that all contracts will not conflict with existing contracts, their contracts will not be merged into the database.

The database will reply that an exception has occurred and it is up to the client application to handle it.
The client library _(not the application)_ should throw an exception or have a status code available.

The client library, _which at this point already has internal registration done_, first registers to the server and obtain its identification and key, then it attempts to register all the contracts it is aware of. 

###Client Unregistration
We detect unregistration normally by message, however we will want to have a try-catch since disconnections, using the C++ API seem to throw the `zmq::error_t` type which is derived from `std::exception`. 

Exception-caused unregistration will be deferred and scheduled, according to configuration but by default maybe **5 minutes** later, giving a large window for reconnecting. The client should have an exception as well, so it should have a mechanism for collecting and trying to connect back.

Proper Unregistration will remove all the markers for _actor dependencies_ in all property sets, and given a flag, will remove all entities under that actor's ownership.

The table of actors **will not** release the ID and key, in case ownership comes back much later. With 64 bit numbers, it is unlikely to exhaust it. 

The entities within the actor's ownership will remain stagnant if the client does not explicitly specify to remove them, it is up to a master controller to devise policy for removing them. 

##Event Resolution
Events are supposed to be distributed to only those that care. I do not want to have mass duplication and validation, in order to to do so, the source that emits needs to have some form of awareness of what will subscriptions exist.

My aim is a tiered awareness at the server and client levels. _Note, the term client refers to the library  that couples with the server, client does not refer to the actual application._

The server will be aware of the property sets the client is interested in. Dependencies will simply be which client to send to. When change events occur, a set is computed being the union of all _owning_ properties for each modified attribute. Using the precalculated dependencies, either a custom generated attribute collection will be formed from the contract which this unioned set has, or the full entity will be sent, depending on the size and what the configuration specifies as a threshold. 
_(For example, suppose there's a descriptor field, which has 512KB of text in it, it wasn't changed, and it is its own property. There's no reason to send it back, nothing will read it.)_
The server will have two different kinds of sets. Data oriented, and event oriented. Data oriented sets are what have references to entities. All sets that have no references are considered event oriented. Event oriented exist to hold a cached set of dependent actors for which to send messages to. In an optimal situation, these are all created on initiation. _I believe it would be ideal to have a logging mechanism in place to record when event oriented sets are created at runtime._
Dependency calculation is iterating a powerset and unioning all the actors from all subsets, this is not a cheap initiation, as seen in the implementation section.

The client will be aware of all the components, dependencies are also precalculated on initiation, however initiation of another component can happen at runtime, _though it is not likely that such will be done except in live development._
A similar pattern is followed with data oriented, and event oriented sets. Instead of actors, component client context references are kept, where a reference to the change is deposited.

I'm thinking of using atomic reference counting on the data portion and passing an immutable reference to all client contexts for them to later process. This supposes that a registered client context will eventually be processed. **For now, concerning sleeping contexts, this will not be considered.** 


##Client

The client library which couples with the database will need to have a few soft stages in its design.

+ Initiate Local Contracts
+ Initiate Potential Event paths _for precalculated event resoluton_
+ Runtime Configuration

Although contract creation and event resolution can happen at runtime, it should be logged and later put integrated into the initiation process as to not cause lag for the subsystem. This is not likely possible to enforce for when other peers to the database which have different directives produce events that are not expected. For a game this behavior is extremely unlikely.

The Client library needs to provide a Master context which connects to the database server / service. This library runs in its own thread. The master context needs to provide contexts meant for component instances, which listen and emit events regarding entities. The Master context needs to be thread safe.

The Component Context, which is a not-necessarily-thread-safe object which belongs to each component instance. It communicates with the master context. It receives information from the thread which the master context resides in. It can emit information to the master context from the thread which the component instance resides in.

It is the assumption that components will eventually be processed, that is, they will processed within a matter of frames. Each client context has its buffer which the master context deposits messages to. It may be considered a memory leak if the client context is never read, it should be destroyed. It is up to the client application to properly handle these. Having a sleep flag would only provide usefulness if during sleep, a record was kept of only unique entity IDs which changed, and then upon wake, it would request those entities back as current change events. **However, this is beyond the scope of this first version.**

There should exist the master context which has the following callable things for general use.

+ Configure
+ Connect and initialize and start,
    _This will throw an exception if there is a conflict with the contracts._
+ Halt,
    _Used to end and clean up._

It can be considered an [Active Object](http://en.wikipedia.org/wiki/Active_object), though most of these are asynchronous.
For use with the component context, it has the following callable things.

+ Register component context
+ Unregister component context
+ Emit entity change
+ Emit Entity Created
+ Emit Entity Destroyed
+ Emit Entity ID change
+ Emit Entity Property Added
+ Emit Entity Property Removed

All the _Emit_ methods also take the identification of the component, which needs to be unique per component per machine (which [UUIDs](http://en.wikipedia.org/wiki/Universally_unique_identifier) would be appropriate for) as to avoid feedback.

This means, at least, that the component contexts should have their own UUID, instead of enforcing the user to provide one.

Furthermore, from the Master Context to the component context, the following callable things need to be present.

+ Receive entity change
+ Receive Entity ID change
+ Receive Entity Property Added
+ Receive Entity Property Removed

_Notice that there's no destruction or creation. These can be synonymous and should be to property additions and subtractions._

From the Component to the component context, the following callable things need to be present.

+ Iterate Entity Changes
+ Iterate Entity Property added
+ Iterate Entity Property removed
+ Iterate Entity ID change

To clarify, the _Property added_ means that the entity has **joined** this specific component's jurisdiction, similar applies to _Property removed_.

Iterating should be **destructive!** Also, the contents being iterated should be thread safe when the master context places new data. Thus the iterating should not be modified externally. I propose swapping storage structures for where the master context dumps to. There is no situation where multiple iterations will be happening in different threads.

> I'm also pondering, since I plan to use reference counting for the actual data, should the destructor happen in the last-owning thread, or should it be placed into a thread-safe queue which is processed in a separate dedicated thread for memory management? It might be easier to debug and log if it is placed in its own thread.


#Implementation and results
##Templated Set
The `cordite::set<T>` class has been made and allows for arbitrary types and chained functions.
Move semantics are available and it is seemingly fast..

The members that go in need to be comparable. 

At first I implemented, incorrectly, a translation of the `C++` example on rosetta code. It doesn't work for sizes > 3. Then I tried doing a for-each based version which the `D` example had.
Although each step I took cut off a digit, it was still not all that efficient, so I came up with a near-binary like version which after analyzation of a simulation of a 5 sized set, I came up with a way to generate sorted combinations of increasing size.

This is what is used now, though the for-each code has been left for educational purposes.

Now that I have powerset working, I should be able to continue on with making the headers for the classes I'm thinking about.

Here are some statistics.. 

    Took avg: __0ms	_____0µs	______917ns		for 1 items.
    Took avg: __0ms	_____6µs	_____6601ns		for 2 items.
    Took avg: __0ms	_____6µs	_____6820ns		for 3 items.
    Took avg: __0ms	____18µs	____18591ns		for 4 items.
    Took avg: __0ms	____78µs	____78385ns		for 5 items.
    Took avg: __0ms	___107µs	___107938ns		for 6 items.
    Took avg: __0ms	___254µs	___254422ns		for 7 items.
    Took avg: __0ms	___390µs	___390977ns		for 8 items.
    Took avg: __0ms	___987µs	___987951ns		for 9 items.
    Took avg: __1ms	__1415µs	__1415170ns		for 10 items.
    Took avg: __2ms	__2861µs	__2861457ns		for 11 items.
    Took avg: __4ms	__4843µs	__4843092ns		for 12 items.
    Took avg: _10ms	_10699µs	_10699752ns		for 13 items.
    Took avg: _22ms	_22643µs	_22643424ns		for 14 items.
    Took avg: _51ms	_51293µs	_51293338ns		for 15 items.
    Took avg: _86ms	_86462µs	_86462578ns		for 16 items.
    Took avg: 192ms	192509µs	192509066ns		for 17 items.
    Took avg: 408ms	408415µs	408415512ns		for 18 items.
    Took avg: 839ms	839028µs	839028897ns		for 19 items
    Took avg: 1861ms		   1861842241ns		for 20 items.
	
Exponential regression yields `1.1029*e^(0.7153x) µs`, where x is the number of items, with an `R^2` of `0.998`.

Memory-wise, it looks like this..

![](http://i.imgur.com/Ik4mc.png)

Which means that I don't have leaks, however my temporaries looks like they could be used better.

Ah... Found the reason! I was preallocating `n^2` instead of `2^n`! And now it looks like
![](http://i.imgur.com/hDmPP.png)

And as you can see, it takes less time for the 4th hump which is the last hump in the fist image, since there's not as much reallocation!

And now it looks like...

    Took avg: ____0ms _______1µs ________1107ns for 1 items.
    Took avg: ____0ms _______3µs ________3033ns for 2 items.
    Took avg: ____0ms _______6µs ________6888ns for 3 items.
    Took avg: ____0ms ______16µs _______16737ns for 4 items.
    Took avg: ____0ms ______36µs _______36742ns for 5 items.
    Took avg: ____0ms ______67µs _______67964ns for 6 items.
    Took avg: ____0ms _____141µs ______141494ns for 7 items.
    Took avg: ____0ms _____420µs ______420110ns for 8 items.
    Took avg: ____0ms _____553µs ______553930ns for 9 items.
    Took avg: ____1ms ____1117µs _____1117726ns for 10 items.
    Took avg: ____2ms ____2632µs _____2632462ns for 11 items.
    Took avg: ____4ms ____4196µs _____4196907ns for 12 items.
    Took avg: ____8ms ____8466µs _____8466593ns for 13 items.
    Took avg: ___16ms ___16738µs ____16738037ns for 14 items.
    Took avg: ___35ms ___35585µs ____35585214ns for 15 items.
    Took avg: ___72ms ___72747µs ____72747781ns for 16 items.
    Took avg: __159ms __159790µs ___159790957ns for 17 items.
    Took avg: __329ms __329911µs ___329911550ns for 18 items.
    Took avg: __658ms __658388µs ___658388587ns for 19 items.
    Took avg: _1359ms _1359292µs __1359292657ns for 20 items.

Which is considerable improvement! `0.8036*e^(.7191x)` is the current trend.

For the iterable powerset, here are the following stats.

    Took avg: ____0ms _______2µs ________2966ns for 1 items.
    Took avg: ____0ms _______2µs ________2967ns for 2 items.
    Took avg: ____0ms _______4µs ________4396ns for 3 items.
    Took avg: ____0ms _______6µs ________6367ns for 4 items.
    Took avg: ____0ms ______10µs _______10459ns for 5 items.
    Took avg: ____0ms ______19µs _______19835ns for 6 items.
    Took avg: ____0ms ______53µs _______53041ns for 7 items.
    Took avg: ____0ms _____101µs ______101283ns for 8 items.
    Took avg: ____0ms _____172µs ______172787ns for 9 items.
    Took avg: ____0ms _____430µs ______430466ns for 10 items.
    Took avg: ____0ms _____921µs ______921839ns for 11 items.
    Took avg: ____1ms ____1874µs _____1874373ns for 12 items.
    Took avg: ____3ms ____3737µs _____3737168ns for 13 items.
    Took avg: ____8ms ____8165µs _____8165788ns for 14 items.
    Took avg: ___16ms ___16678µs ____16678161ns for 15 items.
    Took avg: ___36ms ___36463µs ____36463787ns for 16 items.
    Took avg: ___81ms ___81322µs ____81322017ns for 17 items.
    Took avg: __160ms __160378µs ___160378532ns for 18 items.
    Took avg: __340ms __340493µs ___340493143ns for 19 items.
    Took avg: __758ms __758227µs ___758227067ns for 20 items.

The trend for iteration is: `0.4056*e^(.7097x)`, 

Here I have the allocation graph for 2 instances of Power set generation, and two instances of powerset iteration, side by side.
The memory foot print is obvious, and the time it takes is also obvious, though because of profiling, the allocation and deallocation times are exaggerated.

![](http://i.imgur.com/1PYaD.png)

According to the tables above, it is obvious that iteration is faster! Though it is still inherently exponential.
The good news is that we don't care about a full instantiated powerset! We only care about iterating over it!

So far, this is good enough, though I'll still want to not do this as often as I can.

##Server
_Progress Pending_

