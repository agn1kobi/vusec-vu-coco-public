from util import ASTTransformer
from fennec_ast import Type, Operator, VarDef, ArrayDef, Assignment, Modification, \
        If, Block, VarUse, BinaryOp, IntConst, Return, While


class Desugarer(ASTTransformer):
    def __init__(self):
        self.varcache_stack = [{}]

    def visitFor(self, node):

        self.visit_children(node)
        
        if isinstance(node.entry, str):
            entry = VarUse(node.entry)
        
        initialAssg = VarDef(Type.get('int'), str(entry), node.lexpr).at(node)
        incrementor = Assignment(entry, BinaryOp(entry, Operator('+'), IntConst(1))).at(node)
        
        whileBody = Block([node.body, incrementor]).at(node)
        whileCond = BinaryOp(entry, Operator("<"), node.rexpr).at(node)
        
        while_stat = While(whileCond, whileBody).at(node)
        while_stat.make_desugar(incrementor)
        
        finalBlock = Block([initialAssg, while_stat]).at(node)
        return finalBlock
    
    def makevar(self, name):
        # Generate a variable starting with an underscore (which is not allowed
        # in the language itself, so should be unique). To make the variable
        # unique, add a counter value if there are multiple generated variables
        # of the same name within the current scope.
        # A variable can be tagged as 'ssa' which means it is only assigned once
        # at its definition.
        name = '_' + name
        varcache = self.varcache_stack[-1]
        occurrences = varcache.setdefault(name, 0)
        varcache[name] += 1
        return name if not occurrences else name + str(occurrences + 1)

    def visitFunDef(self, node):
        self.varcache_stack.append({})
        self.visit_children(node)
        self.varcache_stack.pop()

    def visitModification(self, m):
        # from: lhs op= rhs
        # to:   lhs = lhs op rhs
        self.visit_children(m)
        return Assignment(m.ref, BinaryOp(m.ref, m.op, m.value)).at(m)
