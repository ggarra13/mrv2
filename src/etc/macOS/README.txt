
To install it by opening the .dmg file, and dragging the mrv2, vmrv2 and optional hdr icon to the Applications directory.  If there's already an mrv2 version, we recommend you overwrite it.

The macOS application is currently not notarized, so when you launch it, you will not be able to run it as macOS will warn you that the file is not secure as it was downloaded from internet.

To avoid that, you need to open the Apple Logo->Settings->Privacy and Security and go to Security and allow "Opening Anyway".

Alternatively, you can do it from the Terminal (/Applications/Terminal.app), by:
  
```
  sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

or:

  sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/

```

if you install the optional hdr.app then you can do:


```
  sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```

------------------------------------------------------------------------------
Um die Anwendung zu installieren, öffnen Sie die .dmg-Datei und ziehen Sie die Symbole von mrv2, vmrv2 und optional hdr in das Verzeichnis „Programme“ (Applications). Falls bereits eine Version von mrv2 installiert ist, empfehlen wir, diese zu überschreiben.

Die macOS-Anwendung ist derzeit nicht notariell beglaubigt (notarized). Daher können Sie sie beim ersten Start nicht direkt ausführen, da macOS Sie warnt, dass die Datei nicht sicher ist, weil sie aus dem Internet heruntergeladen wurde.

Um dies zu umgehen, öffnen Sie Apple-Menü → Systemeinstellungen → Datenschutz & Sicherheit, gehen Sie zum Abschnitt „Sicherheit“ und erlauben Sie „Dennoch öffnen“.

Alternativ können Sie dies im Terminal (/Applications/Terminal.app) ausführen:

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

oder:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

Wenn Sie zusätzlich hdr.app installieren, können Sie Folgendes ausführen:

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```
-----------------------------------------------------------------------
Para instalar la aplicación, abra el archivo .dmg y arrastre los iconos de mrv2, vmrv2 y, opcionalmente, hdr al directorio Aplicaciones. Si ya existe una versión de mrv2 instalada, le recomendamos sobrescribirla.

Actualmente la aplicación para macOS no está notarizada, por lo que cuando intente ejecutarla no podrá hacerlo directamente, ya que macOS le advertirá que el archivo no es seguro porque fue descargado de Internet.

Para evitarlo, abra el menú Apple → Configuración → Privacidad y Seguridad, vaya a la sección Seguridad y permita la opción «Abrir de todos modos».

Como alternativa, puede hacerlo desde la Terminal (/Applications/Terminal.app) ejecutando:

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

o:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

Si instala también hdr.app de forma opcional, puede ejecutar:

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```
------------------------------------------------------------------------------
इसे इंस्टॉल करने के लिए .dmg फ़ाइल खोलें और mrv2, vmrv2 तथा वैकल्पिक hdr आइकन को Applications निर्देशिका में खींचकर छोड़ दें। यदि mrv2 का कोई संस्करण पहले से मौजूद है, तो हम उसे ओवरराइट करने की सलाह देते हैं।

macOS अनुप्रयोग वर्तमान में notarized नहीं है, इसलिए जब आप इसे चलाने का प्रयास करेंगे, तो macOS आपको चेतावनी देगा कि फ़ाइल सुरक्षित नहीं है क्योंकि इसे इंटरनेट से डाउनलोड किया गया है।

इससे बचने के लिए Apple लोगो → Settings → Privacy & Security खोलें, Security अनुभाग में जाएँ और "Open Anyway" की अनुमति दें।

वैकल्पिक रूप से, आप इसे Terminal (/Applications/Terminal.app) से निम्न आदेश चलाकर कर सकते हैं:

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

या:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

यदि आप वैकल्पिक hdr.app भी इंस्टॉल करते हैं, तो आप यह चला सकते हैं:

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```

-------------------------------------------------------------------------
Per installare l'applicazione, aprire il file .dmg e trascinare le icone di mrv2, vmrv2 e dell'eventuale hdr nella cartella Applicazioni. Se è già presente una versione di mrv2, si consiglia di sovrascriverla.

L'applicazione macOS non è attualmente notarizzata, quindi al primo avvio non sarà possibile eseguirla direttamente perché macOS mostrerà un avviso indicando che il file non è sicuro poiché è stato scaricato da Internet.

Per evitare questo problema, aprire Menu Apple → Impostazioni → Privacy e Sicurezza, andare alla sezione Sicurezza e consentire l'opzione "Apri comunque".

In alternativa, è possibile farlo dal Terminale (/Applications/Terminal.app) eseguendo:

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

oppure:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

Se si installa anche hdr.app opzionalmente, è possibile eseguire:

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```

----------------------------------------------------------------------------
インストールするには、.dmg ファイルを開き、mrv2、vmrv2、およびオプションの hdr のアイコンを「Applications」フォルダへドラッグしてください。すでに mrv2 がインストールされている場合は、上書きすることをお勧めします。

現在、この macOS アプリケーションは notarization（公証）されていないため、起動時に macOS が「インターネットからダウンロードされたため安全性を確認できない」と警告し、そのままでは実行できません。

これを回避するには、Apple メニュー → 設定 → プライバシーとセキュリティ を開き、「セキュリティ」セクションで「このまま開く」を許可してください。

または、ターミナル (/Applications/Terminal.app) で次のコマンドを実行することもできます。

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

または:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

オプションの hdr.app をインストールした場合は、次のコマンドを実行してください。

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```
---------------------------------------------------------------------------
Para instalar o aplicativo, abra o arquivo .dmg e arraste os ícones do mrv2, vmrv2 e, opcionalmente, do hdr para o diretório Aplicativos. Se já existir uma versão do mrv2 instalada, recomendamos substituí-la.

O aplicativo para macOS atualmente não é notarizado, portanto, ao iniciá-lo, o macOS exibirá um aviso informando que o arquivo não é seguro porque foi baixado da Internet.

Para evitar isso, abra o menu Apple → Ajustes → Privacidade e Segurança, vá até a seção Segurança e permita a opção "Abrir Mesmo Assim".

Como alternativa, você pode fazer isso pelo Terminal (/Applications/Terminal.app) executando:

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

ou:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

Se você também instalar o hdr.app opcional, poderá executar:

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```
--------------------------------------------------------------------------
Чтобы установить приложение, откройте файл .dmg и перетащите значки mrv2, vmrv2 и, при необходимости, hdr в каталог Applications. Если версия mrv2 уже установлена, рекомендуется заменить её новой.

В настоящее время приложение для macOS не прошло процедуру notarization (нотариальной проверки), поэтому при запуске macOS предупредит, что файл небезопасен, так как был загружен из Интернета, и не позволит сразу его открыть.

Чтобы обойти это ограничение, откройте меню Apple → Настройки → Конфиденциальность и безопасность, перейдите в раздел «Безопасность» и разрешите действие «Открыть всё равно».

Также это можно сделать через Терминал (/Applications/Terminal.app), выполнив:

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

или:

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

Если вы также установили дополнительное приложение hdr.app, выполните:

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```
-----------------------------------------------------------------------------
要安装本应用，请打开 .dmg 文件，然后将 mrv2、vmrv2 以及可选的 hdr 图标拖放到“Applications（应用程序）”目录中。如果系统中已经存在 mrv2 版本，我们建议直接覆盖安装。

当前 macOS 应用尚未完成公证（Notarization），因此首次启动时，macOS 会提示该文件不安全，因为它是从互联网下载的，并阻止其直接运行。

要解决此问题，请打开 Apple 菜单 → 设置 → 隐私与安全性，进入“安全性”部分，然后允许“仍要打开”。

或者，您也可以在终端（/Applications/Terminal.app）中执行以下命令：

```
sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/

或者：

sudo xattr -rd com.apple.quarantine /Applications/vmrv2.app/
```

如果您还安装了可选的 hdr.app，则可以执行：

```
sudo xattr -rd com.apple.quarantine /Applications/hdr.app/
```

