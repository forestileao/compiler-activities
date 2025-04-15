mod lexer;
mod parser;
mod code_gen;

fn main() {
    let code = r#"
        int a = 32321;
        int b = 332;

        if (a > b) {
            print("A é maior q B");

            if (a > 0) {
                print("a é maior q 10");
            } else {
                if (a < 10) {
                    print("a é menor q 10");
                } else {
                    print("a é igual a 10");
                }
            }
        } else {
            print("A é menor q B");
        }
    "#;

    let tokens = lexer::lex(code);
    let ast = parser::parse(&tokens);
    let generated_code = code_gen::generate_code(&ast);

    // salva o código gerado em um arquivo e compila
    std::fs::write("generated_code.c", &generated_code).expect("Unable to write file");
    std::process::Command::new("gcc")
        .arg("generated_code.c")
        .arg("-o")
        .arg("generated_code")
        .output()
        .expect("Failed to compile generated code");

    // executa o código gerado
    let output = std::process::Command::new("./generated_code")
        .output()
        .expect("Failed to execute generated code");

    println!("{}", String::from_utf8_lossy(&output.stdout));

}
