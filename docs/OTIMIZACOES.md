# Otimizações Android — 17/10/2026

Base pública: https://github.com/levighost8192/Galaxy-Android-BR-
Commit original: `4ebb96c0916e05c1e829102c2eb1d7f92acf9735`.
Alterações deste fork na branch `otimizacoes-android`.

## Resolução

- O menu oferece 240p, 360p, 480p, 720p, 1080p e 1440p. As opções alteram a altura
  real do alvo de renderização, tanto no Vulkan como no OpenGL ES.
- Novas configurações começam em 360p. Os IDs antigos de 480p a 1440p continuam
  iguais; arquivos de configuração existentes conservam a resolução escolhida.
- O modo de economia limita resoluções altas a 720p e preserva 240p/360p/480p.
- As notas dos APKs BR 2.0/3.0 anunciam funções que não constam nesses fontes
  públicos. Este fork implementa as opções acima; não reproduz funções cujo
  código não foi publicado.

## Vulkan e interface

- O estado de pipeline, descritores de texturas, viewport e scissor só é enviado
  novamente ao driver quando muda. O cache é local ao command buffer e é
  invalidado nos comandos de cópia do EFB.
- O buffer de índices é ligado uma vez por quadro; cada desenho usa `firstIndex`
  para selecionar seus índices, mantendo os mesmos deslocamentos de vértices.
- Texturas alteradas reutilizam imagem, memória e view quando dimensões, mipmaps
  e dispositivo são compatíveis. Mudanças de tamanho recriam a alocação.
- Uploads de texturas compartilham submissão/espera por lote. Cada lote permite
  até 128 alocações temporárias e 8 MiB de dados; uma textura maior é enviada
  sozinha. Esse limite de dados não representa toda a memória usada pelo driver.
- Os oito buffers dinâmicos de cada recurso de quadro permanecem mapeados.
  Uma falha ao mapear uma substituição preserva a alocação anterior.
- Descritores de buffers só são reescritos quando seus buffers mudam.
- Os vetores de vértices da interface Vulkan/OpenGL ES conservam sua capacidade
  entre quadros.
- Os painéis desligado e somente FPS não abrem arquivos de CPU/RAM/GPU.
  O painel completo mede CPU com `CLOCK_PROCESS_CPUTIME_ID` e RAM com o tamanho
  real de página do sistema, inclusive em aparelhos com páginas de 16 KiB.

As cercas e barreiras de sincronização foram preservadas. Os testes simulados
validam o comportamento do código, mas não medem FPS nem validam drivers reais.

## Compilação

O workflow [android-apk.yml](../.github/workflows/android-apk.yml) compila ARM64,
executa os testes e verifica assinatura, alinhamento e conteúdo do APK. O APK
anterior desta branch já foi compilado com sucesso pelo GitHub Actions.
Cada compilação recebe uma chave de teste temporária, portanto não atualiza
uma instalação assinada com outra chave.

Para compilar no PC com as dependências instaladas, execute o script
[Compilar-Galaxy-Android.ps1](../scripts/Compilar-Galaxy-Android.ps1), que usa WSL.
A saída no clone é `out-mobile/GalaxyAndroid.apk`.
Os arquivos do jogo não são incluídos.

## Verificação

- 6 testes existentes de geração de shaders.
- 4 testes de host: telemetria, resoluções, ciclo de vida de buffers/texturas e
  cache de comandos Vulkan.
- As verificações cobrem IDs salvos, alturas de renderização, modo de economia,
  reutilização e redimensionamento de texturas, falha de mapeamento, limites dos
  lotes e invalidação do cache entre cópias/quadros.

Para repetir em Linux, use g++ e os headers oficiais Vulkan, OpenGL ES e EGL:

```bash
export GALAXY_VULKAN_INCLUDE=/caminho/Vulkan-Headers/include
export GALAXY_GL_INCLUDE=/caminho/OpenGL-Registry/api
export GALAXY_EGL_INCLUDE=/caminho/EGL-Registry/api
python3 -m unittest discover -s tools/tests -p test_shadergen.py -v
python3 -m unittest discover -s tools/tests -p test_mobile_performance.py -v
```

No celular, compare a mesma fase com as mesmas configurações, antes e depois.
Teste 240p/360p, transições de fase, pausa/retomada e troca Vulkan/OpenGL ES.
Confirme imagem, áudio e estabilidade após alguns minutos. Os ganhos no
Snapdragon 685 ainda precisam ser medidos no aparelho.
