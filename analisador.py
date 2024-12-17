import json

# Structure to represent token
class Token:
    def __init__(self, type_, value):
        self.type = type_
        self.value = value

    def __repr__(self):
        return f"Token({self.type}, {self.value})"


# Structure to represent a Node in the Syntax Tree
class Node:
    def __init__(self, type_, value=""):
        self.type = type_
        self.value = value
        self.children = []

    def add_child(self, child):
        self.children.append(child)

    def print_tree(self, level=0):
        indent = "  " * level
        tree_representation = f"{indent}{self.type}({self.value})\n"
        for child in self.children:
            tree_representation += child.print_tree(level + 1)
        return tree_representation


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.current = 0

    def current_token(self):
        return self.tokens[self.current] if self.current < len(self.tokens) else None

    def eat(self, type_):
        token = self.current_token()
        if token and token.type == type_:
            self.current += 1
            return token
        raise RuntimeError(f"Unexpected token: {token}, expected: {type_}.")

    def parse(self):
        program_node = Node("Program")
        self.eat("PROGRAM_START")
        while self.current_token() and self.current_token().type not in {"PROGRAM_END", "PROGRAM_FINISH"}:
            program_node.add_child(self.parse_statement_or_block())
        if self.current_token().type == "PROGRAM_END":
            self.eat("PROGRAM_END")
        elif self.current_token().type == "PROGRAM_FINISH":
            self.eat("PROGRAM_FINISH")
        else:
            raise RuntimeError(f"Unexpected token: {self.current_token()}, expected: PROGRAM_END or PROGRAM_FINISH.")
        return program_node

    def parse_statement_or_block(self):
        token = self.current_token()
        if token.type == "BLOCK_START":
            return self.parse_block()
        elif token.type == "TYPE":
            return self.parse_variable_definition()
        elif token.type == "IDENTIFIER":
            return self.parse_assignment()
        elif token.type == "IF_CONDITIONAL":
            return self.parse_if_conditional()
        elif token.type == "DATA_OUTPUT":
            return self.parse_output()
        elif token.type == "DATA_INPUT":  
            return self.parse_input()
        elif token.type == "WHILE_LOOP":  
            return self.parse_while_loop()
        else:
            raise RuntimeError(f"Syntax error: Unexpected token {token}.")

    def parse_block(self):
        block_node = Node("Block")
        block_node.add_child(Node("BlockStart", self.eat("BLOCK_START").value))  

        while self.current_token() and self.current_token().type != "BLOCK_END":
            block_node.add_child(self.parse_statement_or_block())

        block_node.add_child(Node("BlockEnd", self.eat("BLOCK_END").value))  
        return block_node


    def parse_variable_definition(self):
        self.eat("TYPE")
        var_definition_node = Node("VariableDefinition")

        while True:
            var_name = self.eat("IDENTIFIER").value
            var_node = Node("Variable", var_name)
            var_definition_node.add_child(var_node)

            if self.current_token() and self.current_token().type == "ARGUMENT_SEPARATOR":
                self.eat("ARGUMENT_SEPARATOR")  
            else:
                break  

        self.eat("COMMAND_END")  
        return var_definition_node

    def parse_assignment(self):
        var_name = self.eat("IDENTIFIER").value
        assign_node = Node("Assignment", var_name)
        self.eat("ASSIGN")
        expr_node = self.parse_expression()
        assign_node.add_child(expr_node)
        self.eat("COMMAND_END")
        return assign_node

    def parse_if_conditional(self):
        self.eat("IF_CONDITIONAL")  
        self.eat("LEFT_PAREN")  
        condition_node = self.parse_expression()  
        self.eat("RIGHT_PAREN")  
        if_node = Node("IfConditional")  
        if_node.add_child(condition_node)  

        if_node.add_child(self.parse_block())  

        if self.current_token() and self.current_token().type == "ELSE_CONDITIONAL":
            self.eat("ELSE_CONDITIONAL")  
            else_node = Node("ElseConditional")
            else_node.add_child(self.parse_block())
            if_node.add_child(else_node)

        return if_node


    def parse_output(self):
        self.eat("DATA_OUTPUT")
        self.eat("LEFT_PAREN")
        output_node = Node("Output")
        output_node.add_child(self.parse_expression())
        self.eat("RIGHT_PAREN")
        self.eat("COMMAND_END")
        return output_node

    def parse_input(self):
        input_node = Node("Input")
        input_node.add_child(Node("Command", self.eat("DATA_INPUT").value))  
        input_node.add_child(Node("LeftParen", self.eat("LEFT_PAREN").value))  
        input_node.add_child(Node("Identifier", self.eat("IDENTIFIER").value))  
        input_node.add_child(Node("RightParen", self.eat("RIGHT_PAREN").value))  
        input_node.add_child(Node("CommandEnd", self.eat("COMMAND_END").value))  
        return input_node
        
    def parse_while_loop(self):
        """Processa um loop WHILE_LOOP."""
        self.eat("WHILE_LOOP")  
        self.eat("LEFT_PAREN")  
        condition_node = self.parse_expression()  
        self.eat("RIGHT_PAREN")  
        block_node = self.parse_block()  

        # Create a node in the Syntax Tree for a loop
        while_node = Node("WhileLoop")
        while_node.add_child(condition_node)  
        while_node.add_child(block_node)  
        return while_node


    def parse_expression(self):
        expr_node = Node("Expression")

        # Processes the left side of the expression
        if self.current_token().type in {"NUMBER", "IDENTIFIER", "LITERAL_STRING"}:
            left = self.eat(self.current_token().type)
            expr_node.add_child(Node("Operand", left.value))
        else:
            raise RuntimeError(f"Syntax error: Unexpected token {self.current_token()} in expression.")

        # Processes the operator (if any)
        if self.current_token() and self.current_token().type in {"GREATER_EQUAL", "LESS", "EQUAL", "GREATER", "LESS_EQUAL", "SUM", "SUBTRACT", "MULTIPLY", "DIVIDE"}:
            operator = self.eat(self.current_token().type)
            expr_node.add_child(Node("Operator", operator.value))

            # Processes the right side of the expression
            if self.current_token().type in {"NUMBER", "IDENTIFIER"}:
                right = self.eat(self.current_token().type)
                expr_node.add_child(Node("Operand", right.value))
            else:
                raise RuntimeError(f"Syntax error: Unexpected token {self.current_token()} after operator.")

        return expr_node


# load the file input.tpa
def load_tokens(filename):
    tokens = []
    with open(filename, "r") as file:
        data = json.load(file)  # Loads the JSON
        for item in data:
            tokens.append(Token(item["type"], item["value"]))
    return tokens


if __name__ == "__main__":
    try:
        # Load the tokens in the input.tpa
        tokens = load_tokens("input.tpa")

        parser = Parser(tokens)

        syntax_tree = parser.parse()

        print("Parsing completed successfully!")
        print(syntax_tree.print_tree())

        # Save the Syntax Tree in the tree.json
        with open("tree.json", "w") as tree_file:
            tree_file.write(syntax_tree.print_tree())
    except RuntimeError as e:
        print(f"Error: {e}")
