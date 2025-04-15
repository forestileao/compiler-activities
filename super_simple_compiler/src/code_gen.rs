use crate::parser::{Expr, Statement};

pub fn generate_code(statements: &[Statement]) -> String {
  let mut code = String::new();
  code.push_str("#include <stdio.h>\n");
  code.push_str("int main() {\n");

  for stmt in statements {
      code.push_str(&generate_statement(stmt));
  }

  code.push_str("return 0;\n");
  code.push_str("}\n");

  code
}

fn generate_statement(stmt: &Statement) -> String {
  match stmt {
      Statement::Declaration { var_type, name, value } => {
          format!("{} {} = {};\n", var_type, name, generate_expression(value))
      }
      Statement::Print(expr) => {
          match expr {
              Expr::StringLiteral(s) => format!("printf(\"{}\\n\");\n", s),
              Expr::Variable(name) => format!("printf(\"%d\\n\", {});\n", name),
              Expr::Number(n) => format!("printf(\"%d\\n\", {});\n", n),
              _ => panic!("print() com tipo não suportado ainda"),
          }
      }
      Statement::If { condition, then_branch, else_branch } => {
          let mut result = format!("if ({}) {{\n", generate_expression(condition));
          for stmt in then_branch {
              result.push_str(&format!("    {}", generate_statement(stmt)));
          }
          result.push_str("} else {\n");
          for stmt in else_branch {
              result.push_str(&format!("    {}", generate_statement(stmt)));
          }
          result.push_str("}\n");
          result
      }
  }
}

fn generate_expression(expr: &Expr) -> String {
  match expr {
      Expr::Number(n) => n.to_string(),
      Expr::Variable(name) => name.clone(),
      Expr::StringLiteral(s) => format!("\"{}\"", s),
      Expr::BinaryOp { left, op, right } => {
          format!("{} {} {}", generate_expression(left), op, generate_expression(right))
      }
  }
}
