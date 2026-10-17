# Compilar o fork pelo GitHub Actions

Workflow: **Build Android APK**, arquivo `.github/workflows/android-apk.yml`.
Ele compila o código da branch escolhida, sem criar tags ou publicar releases.

1. Envie as alterações para `otimizacoes-android` (ou `main`/`master`);
   o build inicia automaticamente para mudanças nos fontes e testes.
   Se o GitHub bloquear Actions por ser um fork, abra a aba Actions e habilite.
2. Para executar novamente: Actions → Build Android APK → Run workflow.
3. Ao terminar, abra a execução e baixe o artifact `GalaxyAndroid-arm64-<run_id>`.
4. Se falhar, baixe `GalaxyAndroid-logs-<run_id>` e consulte o primeiro erro.

O workflow usa Linux x86_64, Java 17, NDK r27d, CMake 3.31.10, Ninja, compilação
com dois jobs e ccache. Os arquivos do instalador/Python já presentes no
repositório são verificados antes da compilação. A primeira execução também
compila shaderc; as próximas podem reutilizar o cache.

Os testes locais usam headers Vulkan modernos, porque o pacote Vulkan padrão
do Ubuntu 22.04 é mais antigo que as extensões de diagnóstico usadas no código.
A API de renderização do aplicativo continua Vulkan 1.0.

Este workflow gera um APK de teste ARM64 com chave temporária. Não exige
secrets, não grava a chave nos artifacts e não modifica a assinatura do workflow
original de releases. Uma chave diferente não atualiza uma instalação existente;
faça backup dos saves antes de qualquer troca de instalação. Para distribuir
atualizações estáveis, configure uma chave persistente conforme `SIGNING.md` e
use o workflow de release. A compilação não demonstra desempenho ou estabilidade
no celular.

A primeira compilação deste fork foi concluída com sucesso na
[execução 39074563139](https://github.com/contatg/Galaxy-Android-BR-fork2/actions/runs/39074563139).
Para alterações posteriores, confira a execução correspondente ao novo commit.
