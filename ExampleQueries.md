## Supported queries

(1) all Objects

MATCH (objs:Object) return objs;

(2) all variables

MATCH (vars:Variables) return vars;

(3) objects of type ServerType

MATCH (objs:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"ServerType"}) return objs

(4) all tempDevices with a reference to a certain property

MATCH (:Variable{NodeId:"SpecialTag"})<-[:HasProperty]-(objs:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"ServerType"}) return objs

(5) all subtypes of an ObjectType

MATCH (types: ObjectType{NodeId:"i=2253", includeSubTypes: "true"}) RETURN types

## (Currently) not supported queries

all references of a Node

MATCH (:Object{NodeId:"MyNode"})--(nodes) return nodes

multiple returns

MATCH (objs)--(vars) return obs, vars

variable length relationships
