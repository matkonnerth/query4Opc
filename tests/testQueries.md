supported queries

(1) all Objects

MATCH (objs:Object) return objs;

(2) all variables

MATCH (vars:Variables) return vars;

(3) objects of type ServerType

MATCH (objs:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"ServerType"}) return objs

(4) all tempDevices with a reference to a certain property

MATCH (:Variable{NodeId:"SpecialTag"})<-[:HasProperty]-(objs:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"ServerType"}) return objs

break it down to
intermediateResults = (obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"MyId"})
Sink = ReferenceFilter( with ReferenceDescriptionMatcher(HasTypeDefinition))

## (Currently) not supported queries

all references of a Node

MATCH (:Object{NodeId:"MyNode"})--(nodes) return nodes

multiple returns

MATCH (objs)--(vars) return obs, vars

all subtypes of an ObjectType

MATCH (:Object)-[:HasTypeDefinition]->(:VECTOR_OF_OBJECTTYPE)