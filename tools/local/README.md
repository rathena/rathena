# rAthena local no Windows

Requer Docker Desktop iniciado com containers Linux.
Execute no PowerShell, na raiz do repositorio:

```powershell
powershell -ExecutionPolicy Bypass -File tools/local/server.ps1 start
powershell -ExecutionPolicy Bypass -File tools/local/server.ps1 status
powershell -ExecutionPolicy Bypass -File tools/local/server.ps1 logs
powershell -ExecutionPolicy Bypass -File tools/local/server.ps1 stop
```

O primeiro inicio baixa as imagens, compila o servidor e inicializa o banco.
Os personagens e contas ficam no volume Docker `rathena-local_database` e
persistem ao parar/reiniciar. Nao remova esse volume para preservar os dados.
Alteracoes no codigo, NPCs ou configuracao entram na imagem ao executar `start`.

## Cliente

- Endereco: `127.0.0.1`; porta de login: `6900`.
- Servidor: `rAthena-Local`, modo Renewal.
- PACKETVER: `20211103` (cliente compativel com essa versao).
- Para criar uma conta, entre uma vez com `seunome_M` ou `seunome_F` e uma
  senha de sua escolha. Nos proximos acessos use apenas `seunome`.
- O cliente do jogo e seus dados nao estao incluidos neste repositorio.
  Configuracao de pacotes/criptografia do executavel deve corresponder ao servidor.

Para compilar para outra data de cliente:

```powershell
$env:PACKETVER = '20211103' # Substitua pela data do seu executavel compativel.
powershell -ExecutionPolicy Bypass -File tools/local/server.ps1 start
```

As portas 6900, 6121 e 5121 sao publicadas apenas em localhost.
O banco nao publica portas no Windows. As credenciais sao exclusivas deste
ambiente local de desenvolvimento; nao use esta configuracao em producao.
Para abrir o console SQL do banco:

```powershell
docker compose -f tools/local/compose.yml exec db mariadb -uragnarok -p ragnarok
```

Senha local do banco: `ragnarok`.
