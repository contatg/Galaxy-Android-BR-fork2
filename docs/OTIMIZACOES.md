# Otimizações Android — 17/10/2026

Base: https://github.com/levighost8192/Galaxy-Android-BR-
Commit original: `4ebb96c0916e05c1e829102c2eb1d7f92acf9735`.

Este pacote contém fontes alterados e testes. Não contém APK compilado nem os
arquivos do jogo. Os ganhos de FPS e consumo ainda precisam ser medidos no celular.

## Alterações aplicadas

- Vulkan: texturas novas/alteradas compartilham uma submissão de comandos por lote,
  em vez de executar `vkQueueSubmit` e `vkQueueWaitIdle` para cada textura. O lote
  limita os dados temporários de upload a 8 MiB; uma textura individual maior é
  permitida e enviada sozinha. Esse limite é de dados, não da memória total do driver.
- Vulkan: os oito buffers dinâmicos de cada recurso de quadro permanecem mapeados.
  O mapeamento só muda ao aumentar a capacidade, e é desfeito antes de liberar
  a memória. Uma falha no novo mapeamento preserva a alocação anterior.
- Vulkan: descritores dos buffers de matrizes, registros, estados e câmera só
  são reescritos quando seus buffers mudam. Atualizar o conteúdo não exige
  reescrever os descritores.
- Interface Vulkan e OpenGL ES: os vetores de vértices conservam sua capacidade
  entre quadros, evitando reconstruir sua alocação a cada desenho.
- Telemetria: os modos desligado e somente FPS deixam de abrir arquivos de
  CPU/RAM/GPU. O modo completo mede CPU do processo com `CLOCK_PROCESS_CPUTIME_ID`,
  sem ler `/proc/stat` ou `/proc/self/stat`. A leitura de RAM usa o tamanho real
  de página informado pelo sistema, incluindo aparelhos com páginas de 16 KiB.
- Falhas nos uploads: mapeamento, início de gravação e submissão são verificados;
  a interface não desenha usando um buffer cujo upload falhou.

As cercas e barreiras de sincronização foram preservadas. Resolução, texturas,
limite de FPS, física, shaders e configurações salvas mantêm os valores existentes.

## Compilar no WSL/Linux

Na pasta dos fontes, com as dependências do README original instaladas:

```bash
export ANDROID_NDK_HOME=/caminho/para/android-ndk
bash build_mobile.sh game main reserve installer
python3 package_mobile_apk.py
```

A geração de `installer` e o empacotamento também dependem do Python Android e
das demais dependências externas indicadas pelo projeto original. Este pacote
não altera o instalador nem fornece essas dependências.

O caminho de saída documentado pelo projeto é `out-mobile/GalaxyAndroid.apk`.
Neste ambiente, a tentativa de compilação parou com:
`Android NDK not found: set ANDROID_NDK_HOME`. Não foi produzido APK.

## Aplicar somente o patch em um clone existente

O arquivo `galaxy-android-otimizacoes.patch`, no topo do ZIP, aplica as mesmas
alterações no commit base. Na raiz do clone:

```bash
git apply --check /caminho/galaxy-android-otimizacoes.patch
git apply /caminho/galaxy-android-otimizacoes.patch
```

Não aplique o patch sobre os fontes deste ZIP: eles já estão alterados.

## Verificação realizada

- 6 testes existentes de geração de shaders passaram.
- 2 testes novos compilaram o renderizador/interface reais em Linux com APIs
  Android/Vulkan simuladas e passaram.
- O teste de upload verificou reutilização e crescimento de buffers, preservação
  da alocação anterior em falha, 100 texturas pequenas em uma única submissão,
  ausência de reenvio de texturas inalteradas, lotes de 8 MiB, limpeza de recursos
  temporários e interrupção do renderizador após falha de submissão.
- O teste de telemetria verificou ausência de aberturas de arquivos nos modos
  desligado/somente FPS e reinício da referência de CPU ao alternar o painel.
- Os testes simulados não validam saída visual, desempenho ou drivers reais.

Para repetir os testes de host em Linux, use g++ e os headers oficiais Vulkan,
OpenGL ES e EGL (instalados no sistema ou nas variáveis abaixo):

```bash
export GALAXY_VULKAN_INCLUDE=/caminho/Vulkan-Headers/include
export GALAXY_GL_INCLUDE=/caminho/OpenGL-Registry/api
export GALAXY_EGL_INCLUDE=/caminho/EGL-Registry/api
python3 -m unittest discover -s tools/tests -p test_shadergen.py -v
python3 -m unittest discover -s tools/tests -p test_mobile_performance.py -v
```

## Validação pendente no celular

Compare antes/depois no mesmo aparelho, mesma fase e mesmas configurações.
Faça uma passagem fria para observar compilação/carregamento, depois repita
com caches aquecidos. Registre FPS, tempo de quadro e consumo após 10 minutos.
Teste transição de fase, pausa/retomada, troca Vulkan/OpenGL ES, edição dos
controles e painel desligado/somente FPS/completo. Confirme imagem, áudio e
estabilidade. Não há promessa de 60 FPS ou percentual de melhoria.
