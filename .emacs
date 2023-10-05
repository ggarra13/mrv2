;;; Gonzalo Garramuno's Emacs init file.
;;; ggarra@advancedsl.com.ar
;;;
;;; This file contains my emacs customizations. Most of this file I have
;;; borrowed from other files, but some(very little) I wrote myself.
;;; gnu.help.emacs has been an invaluable resource.
;;;
;;; This file has been tested to work fine under both GNUEmacs 21.3.1
;;; and XEmacs 21.4.
;;; Note, however, that for full functionality, it may require additional
;;; modules
;;;
;;; This file relies on additional emacs lisp modules that are not shipped
;;; with emacs.  If those modules are not present, this .emacs file will
;;; not fail, but just spit out a message about it in *Message* window
;;; when it boots.
;;;
;;; Current packages used are:
;;;
;;; Programs that need separate installation:
;;;   aspell       Program used to run ispell and do spell checking.
;;;
;;; Other:
;;;   color-theme   Allows you to set emacs colors to certain "themes".
;;;   mic-paren     An extension to parenthesis hiliting that does a better job
;;;
;;; Programming:
;;;   bat-generic   Edit .bat files
;;;   rd            Mode to edit Ruby's rdoc documentation
;;;   ruby          Mode to edit Ruby code
;;;   ruby-electric Mode for Ruby that adds matching end/brackets/etc to code
;;;   rsl           Mode to edit Renderman's SL code.
;;;   php           Mode to edit PHP code
;;;   pabbrev       Mode to use aggressive completion of keywords
;;;   visual-basic  Mode to edit Visual Basic programming language
;;;   javascript    Mode to edit Javascript programming language
;;;
;;; This file is released under the BSD license.
;;;
;;; LAST MODIFIED BY:  Gonzalo Garramuno

;;;;;;;;;;;;;;;;;;;;
;; Macros to use around code that runs only on GNUEmacs or XEmacs only.
;;

;; Added by Package.el.  This must come before configurations of
;; installed packages.  Don't delete this line.  If you don't want it,
;; just comment it out by adding a semicolon to the start of the line.
;; You may delete these explanatory comments.

(require 'package)
(add-to-list 'package-archives
             '("melpa" . "https://melpa.org/packages/") t)
(package-initialize)

(defmacro EmacsVersion( v &rest x )
  (list 'if (string-match v (version)) (cons 'progn x)))
(defmacro EmacsNotVersion( v &rest x )
  (list 'if (not (string-match v (version))) (cons 'progn x)))
(defmacro GNUEmacs (&rest x)
  (list 'if (string-match "GNU Emacs" (version)) (cons 'progn x)))
(defmacro XEmacs (&rest x)
  (list 'if (string-match "XEmacs" (version)) (cons 'progn x)))


(defun gg-require(x)
  "Require an emacs module.  Log a message if not found and return nil."
  (condition-case err
      (require x)
    (error
      (message "Emacs library %s not loaded: %s" x (cdr err))
      nil
      )
    )
)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; various setups
;;;

;;
;; Start-up emacs as a server.
;; On windows, use gnuserv.  Other platforms, use server-start natively.
;; Reason for this that server.el has not really been ported to windows, but
;; a non-standard port of it called gnuserver exists on windows.
;;
;; The main purpose for this is to have a single instance of
;; Emacs. Files are sent to "emacsclient" or "gnuclientw" instead of
;; Emacs which prevents numerous Emacs frames from opening all over the
;; place. This is required for things such as running emacs as the
;; default MSVC editor using VisEmacs. I usually start emacs once in
;; the morning, then keep it running all day as my only editor.
(defun my-server-start()
  "Function to safely start an emacs server"
  (if (eq system-type 'windows-nt)
      (if (gg-require 'gnuserv)
	  (gnuserv-start))
    (server-start)
    )
  )

;;; I was having issues with the gnuserver working in XEmacs on windows,
;;; so we wil;; l make it work for GNUEmacs only for now.
;; (GNUEmacs
;;  (condition-case err
;;      (my-server-start)
;;    (error
;;     (message "Cannot start emacs server %s" (cdr err))))
;; )

(setq gnuserv-frame (selected-frame)) ; Make sure we don't open new windows

;;; Use spaces instead of tabs to indent
(setq cmake-tab-width 4)
(setq tab-width 4)
(setq indent-tabs-mode nil)
;;;

(GNUEmacs
 (tool-bar-mode -1)
 )

;;; return to same line on a scroll back
(setq scroll-preserve-screen-position t)
;;;

;;; Have the titlebar contain "user@machine   buffer  filename"
;;; This is relatively neat if you minimize the window, as it leaves
;;; something like: gga@pokey in the iconized title-bar
(setq frame-title-format '("" user-login-name "@" system-name "  %b  " buffer-file-name))
;;;


;;; Paste at cursor NOT mouse pointer position
(setq mouse-yank-at-point t)
;;;

;;; Type 'y' instead of 'yes'
(fset 'yes-or-no-p 'y-or-n-p)
;;;

;;; Turn off backup files.
;;; (setq make-backup-files nil)
;;;

;;; Display the time on modeline
;; (condition-case err
;;     (display-time)
;;   (error
;;    (message "Cannot display time %s" (cdr err))))
;; ;;;

;;; Display the column number on modeline
(condition-case err
    (column-number-mode t)
   (error
	(message "Cannot display Column Number %s" (cdr err))))
;;;

;;; This is to not display the initial message (which says to a novice
;;; user what to do first if he/she is confused).
(setq inhibit-startup-message t)
;;;

;;; ...and this inhibits the startup blurb in the echo area.
(setq inhibit-startup-echo-area-message t)
;;;

;;; This is to make emacs avoid making backups like test.cpp~
(setq make-backup-files nil)

;;; This disables down-arrow and C-n at the end of a buffer from adding
;;; a new line to that buffer.
(setq next-line-add-newlines nil)
;;;

;;; Overlap between window-fulls when scrolling by pages
(setq next-screen-context-lines 2)
;;;

;;; Turn on auto-fill and limit editing to 80 characters in the terminal.
(setq auto-fill-mode t)
(setq-default auto-fill-mode t)
(setq current-fill-column 80)
;;;

;;; Handle .gz/zip/lha/etc files
(auto-compression-mode t)
;;;

;;; Allow completions like em-s-region to complete to emacspeak-speak-region
;;;(GNUEmacs
;;;(partial-completion-mode)
;;;)
;;;

;;; Get rid of old versions of files
(setq delete-old-versions t)
;;;

;;; Highlight during query
(setq query-replace-highlight t)
;;;

;;; Highlight incremental search
(setq search-highlight t)
;;;

;;; Scroll stuff
(setq scroll-conservatively 5)
(setq scroll-step 1)

;;; Make sure when marking stuff for Yanking, we show the selection, as we
;;; do with mouse
(setq-default transient-mark-mode t)
;;;

;;; Set for visual bell.  This will make emacs flash the screen instead of
;;; beeping in case of an error.
(setq visible-bell t)
;;;

;;; Line number mode true
(setq line-number-mode t)
;;;

;;; Start off in "C:/" dir... for use with pc-emacs
;; (cd "C:/")
;;;


;;; Make searches case insensitive
(setq case-fold-search t)
;;;

;;; Display time in the mode line
(setq display-time-day-and-date nil)
;;; Control window size, position, and font.
;; (set-frame-height (selected-frame) 40)
;; (set-frame-width (selected-frame) 80)
;; (set-frame-position (selected-frame) 10 30)
;; (set-default-font "-*-courier-normal-r-*-*-13-*-*-*-c-*-*-iso8859-1")
;; (set-face-font 'italic "7X14")
;;;
;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Modeline stuff
;;
;; Modeline is the bottom line on emacs showing the status of things.
;;
;; Set a Mode Line that tells me which machine, which directory,
;; and which line + column I am on, plus directory of file.

;;; Show current function in modeline
;; (which-func-mode nil)
;;;

(setq default-mode-line-format
      '("-"
       mode-line-mule-info
       mode-line-modified
       mode-line-frame-identification
       mode-line-buffer-identification
       " "
       global-mode-string
       "   %[(" mode-name mode-line-process minor-mode-alist "%n" ")%]--"
       (line-number-mode "L%l--")
       (column-number-mode "C%c--")
       (-3 . "%p")
       "--"
       default-directory
       "-%-"))
;;;

;;;
(add-to-list 'load-path "~/.emacs.d/share/emacs/site-lisp" )


;;; Automatically makes the matching paren stand out in color.
(condition-case err
    (show-paren-mode t)
  (error
   (message "Cannot show parens %s" (cdr err))))

(if (gg-require 'mic-paren)  ;; enhancement to the parenthesis matching
   ( (paren-activate)
     (add-hook 'c-mode-common-hook
	       (function (lambda ()
			   (paren-toggle-open-paren-context 1))))
     )
)
;;;

;;; This moves the mouse pointer out of my way when I type.
;(condition-case err
;    (mouse-avoidance-mode 'banish)
;  (error
;   (message "Cannot use mouse-avoid %s" (cdr err))))
;;;

;;;  Some self-explanatory settings
(setq auto-save-timeout 1800)  ; Autosave backup
(setq require-final-newline t) ; Make sure a file always ends with newline
(setq compilation-window-height 12)
(setq compilation-ask-about-save nil)
;;;

;;; This makes `apropos' and `super-apropos' do everything that they can.
;;; Makes them run 2 or 3 times slower.  Set this non-nil if you have a
;;; fast machine.
(setq apropos-do-all t)
;;;

;;; Customizes languages support, based on file extension.
(setq auto-mode-alist
      (append '(
		("\\.[Cc][Oo][Mm]\\'" . text-mode)
		; ("\\.bat\\'" . bat-generic-mode)
		("\\.inf\\'" . inf-generic-mode)
		("\\.rc\\'" . rc-generic-mode)
		("\\.reg\\'" . reg-generic-mode)
		("\\.cg$"  . c++-mode)
		("\\.CC$"  . c++-mode)
		("\\.cc$" . c++-mode)
		("\\.glsl$" . c++-mode)
		("\\.yml" . yaml-mode)
		("\\.yaml" . yaml-mode)
		("\\.inl$" . c++-mode)
		("\\.xs$" . c++-mode)
		("\\.cpp$" . c++-mode)
		("\\.cxx$" . c++-mode)
		("\\.hpp$" . c++-mode)
		("\\.hxx$" . c++-mode)
		("\\.hh$" . c++-mode)
		("\\.c$"  . c-mode)       ; to edit C code
		("\\.h$"  . c++-mode)     ; to edit C++ code
		("\\.cs$" . c++-mode)     ; to edit C# code
		("\\.ctl$"  . c++-mode)     ; to edit CTL code
		("\\.i$"  . c++-mode)     ; to edit SWIG code
		("\\.swg$"  . c++-mode)     ; to edit SWIG code
		("\\.m$"  . objc-mode)    ; to edit ObjC code
		("\\.jsp$" . java-mode)   ; to edit java code
		("\\.java$" . java-mode)   ; to edit java code
		("\\.po\\'\\|\\.po\\." . po-mode) ; edit po mode
		("\\.prolog$" . c++-mode) ; to edit prolog code
		("\\.py$" . python-mode)    ; to edit python code
		("\\.sl$" . rsl-mode)     ; to edit Renderman SL code
		("\\.rb$" . ruby-mode)    ; to edit ruby code
		("\\.rbw$" . ruby-mode)    ; to edit ruby code
		("\\.rdoc$" . rd-mode)    ; to edit ruby rdocs
		("\\.rd$" . rd-mode)    ; to edit ruby rdocs
		("^Makefile.in$" . makefile-mode)  ; to edit makefiles
		("\\.make$" . makefile-mode)  ; to edit makefiles
		("\\.mk$" . makefile-mode) ; to edit makefiles
		("\\CONFIG$" . makefile-mode) ; to edit "CONFIG" files
		("cshrc" . shell-script-mode) ; to edit .cshrc related files
		(".emacs" . emacs-lisp-mode) ; to edit emacs related files
		("\\.tcsh$" . shell-script-mode) ; to edit tc-shell scripts
		("\\.bash$" . shell-script-mode) ; to edit bash shell scripts
		("^configure$" . shell-script-mode) ; to edit shell scripts
		("\\.sh$" . shell-script-mode) ; to edit shell scripts
		("\\.csh$" . shell-script-mode) ; to edit c-shell scripts
		("\\.tar$" . tar-mode)  ; to edit tarfiles
		("\\.bas$" . visual-basic-mode) ; to edit visual basic
		("\\.cls$" . visual-basic-mode) ; to edit visual basic
		("\\.frm$" . visual-basic-mode) ; to edit visual basic
		("\\.vb$" . visual-basic-mode) ; to edit visual basic
		) auto-mode-alist))
;;;

;;;
;;; Enable some commands we need.
(put 'narrow-to-region 'disabled nil)
(put 'downcase-region 'disabled nil)
(put 'upcase-region 'disabled nil)
;;;

;;;Setting this to t makes scrolling faster, but may momentarily present
;;;unfontified areas when you scroll into them.
;(setq lazy-lock-defer-driven f)
;(setq lazy-lock-defer-on-scrolling f)
;;;

;;;This enables archive browsing and editing.
(setq auto-mode-alist
      (cons '("\\.\\(arc\\|zip\\|lzh\\|zoo\\)\\'" . archive-mode)
	    auto-mode-alist))
;;;

;;;This enables automatic resizing of the minibuffer when its contents
;;;won't fit into a single line.
(condition-case err
    (resize-minibuffer-mode 1)
  (error
   (message "Cannot resize minibuffer %s" (cdr err))))
;;;

;;; Ps-Print customization.
(setq ps-header-lines 3)
(setq ps-paper-type 'ps-a4)
;;;

;;;
(setq circulate-include "")
;;; C-x C-e runs the command eval-last-sexp:


;;;
;; (setq blink-matching-paren-on-screen t)
;;;


;; Tired of Ctrl-Z minimizing the emacs window.  Disable it.
(when window-system
	(global-unset-key "\C-z")
) ; iconify-or-deiconify-frame (C-x C-z)


;;; end - various setups
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; font-lock (Font Coloring) section

;;; Set up support for lazy-lock
;; (setq font-lock-support-mode t)
;;;

;;; Set the maximum buffer size for coloured text to unlimited
(setq-default font-lock-maximum-size nil)
;;;

;;;

;;;--------------------------------
;;; face font-lock
;;;
;;; [colors] gold,orange,gray,cyan,magenta,brown,pink,light*,dark*
;;;-------------------------------
;;; where FACE should be one of the face symbols, and the subsequent element
;;; items should be the attributes for the corresponding Font Lock mode
;;; faces.  Attributes FOREGROUND and BACKGROUND should be strings (default
;;; if nil), while BOLD-P, ITALIC-P, and UNDERLINE-P should specify the
;;; corresponding face attributes (yes if non-nil).
;;;
;;; (FACE FOREGROUND BACKGROUND BOLD-P ITALIC-P UNDERLINE-P)
;;;
;;; To see all color names, use list-colors-display
;;; To see all font configurations, use list-faces-display

;;; Emacs 21.3.1 broke this
(EmacsNotVersion "21.3.1"
		 (set-face-foreground 'default           "white")
		 (set-face-background 'default           "black")
)

(GNUEmacs
 (set-cursor-color "red")
 (set-foreground-color "white")
 (set-background-color "black")
)

(GNUEmacs
 (set-face-foreground 'region "wheat")
 (set-face-background 'region "darkslategray")
 (set-face-italic-p   'region nil)
)

(set-face-foreground 'highlight  "red")
(set-face-background 'highlight  "Blue4")

(set-face-foreground 'secondary-selection "yellow")
(set-face-background 'secondary-selection "MidnightBlue")

;; (set-face-foreground 'modeline          "gold")
;; (set-face-background 'modeline          "darkblue")

(GNUEmacs
 ;; (set-face-foreground 'show-paren-match-face "white")
 ;; (set-face-background 'show-paren-match-face "blueviolet")
 ;;(set-face-bold-p 'show-paren-match-face t)
 ;; (set-face-foreground 'show-paren-mismatch-face "black")
 ;; (set-face-background 'show-paren-mismatch-face "yellow")
 ;;(set-face-italic-p 'show-paren-mismatch-face t)
 (setq font-lock-face-attributes
      '(
	(font-lock-comment-face        "SteelBlue" nil nil nil nil)
	(font-lock-string-face         "cyan" nil nil nil nil)
	(font-lock-keyword-face        "darkorange"  nil nil nil nil)
	(font-lock-type-face           "yellow" nil nil nil nil)
	(font-lock-function-name-face  "DeepSkyBlue"  nil nil nil nil)
	(font-lock-variable-name-face  "magenta" nil nil nil nil)
	(font-lock-builtin-face        "lightblue" nil nil nil nil)
	(font-lock-constant-face       "cornflowerblue" nil nil nil nil)
	))
)
;;;

;;; Turn on font-locking for various modes.
;; (global-font-lock-mode '(shell-script-mode
;;                          fundamental-mode
;;                          text-mode
;;                          makefile-mode))
;;;

;;; Turn font-locking for all modes that support it
;; (GNUEmacs
;;  (global-font-lock-mode t)
;; )
(XEmacs
 (font-lock-mode t)
)
;;;

;;; Color source code files as much as possible
;; (can be a tad slower on big files)
(setq font-lock-maximum-decoration t)
;; (setq font-lock-maximum-size
;;       (if font-lock-maximum-decoration (* 70 1024) (* 150 1024)))
;;;


;;; end - font-lock section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; load/require section

(gg-require 'yaml-mode)  ;; used for editing yaml files
(gg-require 'clang-format)  ;; used for editing C++ with clang-format
(global-set-key [C-tab] 'clang-format-region)

;; Use LF line endings by default
(setq-default buffer-file-coding-system 'utf-8-unix)
(setq-default default-buffer-file-coding-system 'utf-8-unix)
(setq-default file-name-coding-system 'utf-8-unix)

;; (gg-require 'fm)       ;  used for formating output of compilation

;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Shell mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set maximum-buffer size for shell-mode.  Useful for large outputs
(setq comint-buffer-maximum-size 10240)

; truncate shell buffer to comint-buffer-maximum-size.
(add-hook 'comint-output-filter-functions 'comint-truncate-buffer)

; will disalllow passwords to be shown in clear text (useful, for example,
; if you use the shell and then, login/telnet/ftp/scp etc. to other machines).
(add-hook 'comint-output-filter-functions 'comint-watch-for-password-prompt)

; will remove ctrl-m from shell output.
(add-hook 'comint-output-filter-functions 'comint-strip-ctrl-m)

;;; Load tar-mode
(autoload 'tar-mode "tar-mode")
;;;


;;; Load Shell-toggle.el
(autoload 'shell-toggle "shell-toggle"
"Toggles between the *shell* buffer and whatever buffer you are
editing."  t)
(autoload 'shell-toggle-cd "shell-toggle"
  "Pops up a shell-buffer and insert a \"cd <file-dir>\" command." t)
;;;

;;; Load archive mode
(autoload 'archive-mode "arc-mode" "Major mode for editing archives." t)
;;;

;;; This enables automatic saving of the Emacs desktop configuration into
;;; a file called `emacs.desktop'.  Should be as near the .emacs file end
;;; as possible.
;;; It then allows reopening emacs with all the files you
;;; edited last.
;; (condition-case err
;;     (progn
;;       (load "desktop")
;;       (desktop-load-default)
;;       (setq history-length 250)
;;       (add-to-list 'desktop-globals-to-save 'file-name-history)
;;       (desktop-read)
;;       (setq desktop-enable t))
;;   (error
;;    (message "Cannot save desktop %s" (cdr err))))
;; (setq desktop-missing-file-warning nil)
;; (setq desktop-buffers-not-to-save
;;       (concat "\\(" "^nn\\.a[0-9]+\\|\\.log\\|(ftp)\\|^tags\\|^TAGS"
;;	      "\\|\\.emacs.*\\|\\.diary\\|\\.newsrc-dribble\\|\\.bbdb"
;;	      "\\)$"))
;; (add-to-list 'desktop-modes-not-to-save 'dired-mode)
;; (add-to-list 'desktop-modes-not-to-save 'Info-mode)
;; (add-to-list 'desktop-modes-not-to-save 'info-lookup-mode)
;;;

;;; end - load/require
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; mode stuff (various different modes)



;; ;;; This defines and sets up my customized c-mode indentation.
(defun my-c-stuff ()
  "My own C stuff.  Customizes indentation style and dynamic abbrevs."
  (modify-syntax-entry ?_ "w")

  (imenu-add-menubar-index)
  (c-set-offset 'defun-open 0 nil)
  (c-set-offset 'defun-block-intro 4 nil)
  (c-set-offset 'statement 0 nil)
  (c-set-offset 'statement-block-intro 4 nil)
  (c-set-offset 'substatement 4 nil)
  (c-set-offset 'statement-case-intro 4 nil)
  (c-set-offset 'statement-case-open 4 nil)
  (c-set-offset 'inclass 4 nil)
  (c-set-offset 'case-label 4 nil)
  (c-set-offset 'inline-open 0 nil)
  (c-set-offset 'access-label -2 nil)
  (c-set-offset 'label 10 nil)

  (set (make-local-variable 'c-indent-level) 0)
  (setq c-electric-pound-behavior '(alignleft))
  (setq c-progress-interval 8);; or else I tend to think it crashed
  (setq c-macro-shrink-window-flag t)
  (setq c-macro-prompt-flag t)
  (setq c-toggle-auto-state t)
  (set (make-local-variable 'dabbrev-case-fold-search) nil)
  (set (make-local-variable 'dabbrev-case-replace) nil)
  (define-key c-mode-map "\C-c %" 'match-paren)
  (define-key c++-mode-map "\C-c %" 'match-paren)

  ;(setq c-tab-always-indent t)
  ;(setq c-basic-offset 0)
  (setq-default c-basic-offset 4
		tab-width 4
		indent-tabs-mode . nil)
  (outline-minor-mode)

  (set (make-local-variable 'compile-command) "make")
)

;;;
;;(add-hook 'c-mode-common-hook 'my-c-stuff)
(add-hook 'c-mode-common-hook 'turn-on-font-lock)
;;;

;; ;;; define extra C types to font-lock
;; (GNUEmacs
;;  (setq c-font-lock-extra-types
;;       (append
;;        '("CHAR" "BOOL" "BYTE" "SOCKET" "boolean" "UINT" "UINT16"
;; "UINT32" "ULONG" "FLOAT" "INT" "INT16" "INT32" "uint" "ulong" "string"
;; "BOOLEAN" "\\sw+_T" "LPCSTR" "LPCTSTR" "HRESULT" "BYTE" "DWORD" )
;;        c-font-lock-extra-types))
;;  (setq c++-font-lock-extra-types
;;        (append
;;	c-font-lock-extra-types
;;	c++-font-lock-extra-types))
;;  )

;; ;;; This is for mental ray shading coding
;; (GNUEmacs
;;  (setq c-font-lock-extra-types
;;       (append
;;        '("miColor" "miScalar" "miVector" "miGeoVector" "miMatrix" "miTag" "color" )
;;        c-font-lock-extra-types))

;;; This is for maya coding
;;  (setq c++-font-lock-extra-types
;;       (append
;;        '( "MAngle" "MDagPath" "MItDag" "MItGeometry" "MFnCamera" "MFnDependencyNode" "MFnDagNode" "MMatrix" "MObject" "MObjectArray" "MPlug" "MPlugArray" "MStatus" "MString" "MVector")
;;        c++-font-lock-extra-types))

;; ;;; Do the same for C++
;;  (setq c++-font-lock-extra-types
;;        (append
;;	c-font-lock-extra-types
;;	c++-font-lock-extra-types))
;;)
;;;

;;; Hungry state
;; (add-hook 'c-mode-common-hook
;;           (function
;;            (lambda ()
;;              (c-toggle-hungry-state t))))
;;;

;;; This does a carridge return after a brackets and semi-colons.
;; (add-hook 'c-mode-common-hook
;;           (function
;;            (lambda ()
;;              (c-toggle-auto-state 1))))
;;;


;;; Makefile stuff
(defun my-makefile-stuff ()
  "My own makefile-mode"

  (setq makefile-tab-after-target-colon t)
  )
;;;
(add-hook 'makefile-mode-hook 'my-makefile-stuff)
(add-hook 'makefile-mode-hook 'turn-on-font-lock)
;;;

;; If the *scratch* buffer is killed, recreate it automatically
;; FROM: Morten Welind
;;http://www.geocrawler.com/archives/3/338/1994/6/0/1877802/
(save-excursion
  (set-buffer (get-buffer-create "*scratch*"))
  (lisp-interaction-mode)
  (make-local-variable 'kill-buffer-query-functions)
  (add-hook 'kill-buffer-query-functions 'kill-scratch-buffer))

(defun kill-scratch-buffer ()
  ;; The next line is just in case someone calls this manually
  (set-buffer (get-buffer-create "*scratch*"))
  ;; Kill the current (*scratch*) buffer
  (remove-hook 'kill-buffer-query-functions 'kill-scratch-buffer)
  (kill-buffer (current-buffer))
  ;; Make a brand new *scratch* buffer
  (set-buffer (get-buffer-create "*scratch*"))
  (lisp-interaction-mode)
  (make-local-variable 'kill-buffer-query-functions)
  (add-hook 'kill-buffer-query-functions 'kill-scratch-buffer)
  ;; Since we killed it, don't let caller do that.
  nil)

;;;;;;;;;;;;;;;;;;;;;;;;
;;; Outline mode
;;;
;;; Allows you to collapse blocks of code to a single line.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;
(defun show-onelevel ()
  "show entry and children in outline mode"
  (interactive)
  (show-entry)
  (show-children))

(defun cjm-outline-bindings ()
  "sets shortcut bindings for outline minor mode"
  (interactive)
  (local-set-key [?\C-,] 'hide-sublevels)
  (local-set-key [?\C-.] 'show-all)
  ;(local-set-key [C-up] 'outline-previous-visible-heading)
  ;(local-set-key [C-down] 'outline-next-visible-heading)
  ;(local-set-key [M-left] 'outline-backward-same-level)
  ;(local-set-key [M-right] 'outline-forward-same-level)
  (GNUEmacs
   (local-set-key [M-up] 'hide-subtree)
   (local-set-key [M-down] 'show-subtree)
   )

  ;(local-set-key "M-up" 'hide-subtree)
  ;(local-set-key "M-down" 'show-subtree)

  )

(add-hook 'outline-minor-mode-hook
	  'cjm-outline-bindings)

;;
;; FFMPEG C code
;;
(c-add-style "ffmpeg"
	     '("k&r"
	       (c-basic-offset . 4)
	       (indent-tabs-mode . nil)
	       (tab-width . 4)
	       ;(show-trailing-whitespace . t)
	       (c-offsets-alist
		(statement-cont . (c-lineup-assignments +)))
	       )
	     )
(setq c-default-style "ffmpeg")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Vi mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;(setq viper-mode t)
;(require 'viper)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Speedbar settings
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Spelling.  Use aspell
;;;;;;;;;;;;;;;;;;;;;;;;;
;; (setq-default ispell-program-name "aspell")

;; (if (gg-require 'ispell)
;;     (progn (setq ispell-menu-map-needed t)
;;	   (setq ispell-dictionary "english")
;;	   (setq ispell-personal-dictionary "~/.ispell-dico-perso")
;;	   )
;;   (message "You might need to install aspell for spelling.")
;;   )

; ;;;;;;;;;;;;;;;;;;;;;;;;;
; ;;; Zip/Unzip file stuff
; ;;;;;;;;;;;;;;;;;;;;;;;;;
; (setq archive-zip-use-pkzip nil)  ; we use cygwin 'unzip' command


;;;;;;;;;;;;;;;;;;;;;
;;; Python language stuff
;;;;;;;;;;;;;;;;;;;;;
(setq python-indent-offset 4)

;;;;;;;;;;;;;;;;;;;;;
;;; Ruby language stuff
;;;;;;;;;;;;;;;;;;;;;
(autoload 'rd-mode "rd-mode" nil t)     ; rdoc mode
(autoload 'ruby-mode "ruby-mode" nil t) ; ruby mode
(add-to-list 'interpreter-mode-alist '("ruby" . ruby-mode))


(defun my-ruby-stuff ()
  "My own ruby-mode"
  (turn-on-font-lock)
  (imenu-add-menubar-index)
  (define-key ruby-mode-map [f1] 'ruby-insert-end)

  (outline-minor-mode)
  (setq outline-regexp " *\\(def \\|class\\|module\\|do \\)")

  (gg-require 'ruby-electric)
  (condition-case err
      (ruby-electric-mode)
    (error err)
    )

  (set (make-local-variable 'compile-command)
       (format "ruby -w %s"
	       (file-name-nondirectory buffer-file-name)))
  )

(add-hook 'ruby-mode-hook 'my-ruby-stuff)

;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ;;; Renderman SL
;; ;;; NOTE: the original rsl-mode does not support hooks.
;; ;;;       I added them.
;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; (autoload 'rsl-mode "rsl-mode" nil t)

;; (defun my-rsl-stuff()
;;   (turn-on-font-lock)
;;   (imenu-add-menubar-index)
;;   ;(outline-minor-mode)
;; )
;; (add-hook 'rsl-mode-hook 'my-rsl-stuff)


;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ;;; Electric buffer - This is yet another way to switch buffers
;; ;;; You use: electric-buffer-list
;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; (defun my-ebuf-stuff ()
;;   "My own Electric Buffer Menu stuff.  Currently binds some
;; convenience keys."
;;   (define-key electric-buffer-menu-mode-map [up] 'previous-line)
;;   (define-key electric-buffer-menu-mode-map [down] 'next-line)
;;   (define-key electric-buffer-menu-mode-map [next] 'scroll-up)
;;   (define-key electric-buffer-menu-mode-map [previous] 'scroll-down)
;;   (define-key electric-buffer-menu-mode-map [left] 'scroll-right)
;;   (define-key electric-buffer-menu-mode-map [right] 'scroll-left))

;; (add-hook 'electric-buffer-menu-mode-hook 'my-ebuf-stuff)
;; ;;;

;; ;;;This fontifies the compilation buffer when compilation exits.

;; (defun my-compilation-finish-function (buf msg)
;;   "This is called after the compilation exits.  Currently just
;; highlights the compilation messages."
;;   (save-excursion
;;     (set-buffer buf)
;;     (font-lock-fontify-buffer)))
;; ;;;

;; ;;;
;; (defun my-compilation-mode-stuff ()
;;   "My own compilation set up"
;;   (setq compilation-finish-function 'my-compilation-finish-function)
;;   (setq compilation-window-height 20)
;;   (setq compilation-ask-about-save t)
;;   (setq compilation-read-command t)
;;   (setq compile-auto-highlight t)
;;   )
;; (add-hook 'compilation-mode-hook 'my-compilation-mode-stuff)
;; (add-hook 'compilation-mode-hook 'fm-start)
;; ;;;


;;
;; Doxymacs stuff 
;;

(defun my-doxymacs-font-lock-hook ()
  (if (or (eq major-mode 'c-mode) (eq major-mode 'c++-mode))
      (doxymacs-font-lock)))


(defun my-doxymacs-stuff ()
  "My own doxymacs-mode"
  (define-key doxymacs-mode-map [f5]
    'doxymacs-insert-function-comment)
  (define-key doxymacs-mode-map [f6]
    'doxymacs-insert-blank-multiline-comment)
  (define-key doxymacs-mode-map [f7]
    'doxymacs-insert-blank-singleline-comment)
  (define-key doxymacs-mode-map [f8]
    'doxymacs-insert-member-comment)
  (define-key doxymacs-mode-map [f9]
    'doxymacs-insert-grouping-comments)
  (define-key doxymacs-mode-map [f10]
    'doxymacs-insert-file-comment)
  )

(if (gg-require 'doxymacs)
    (progn (add-hook 'c-mode-common-hook 'doxymacs-mode)
	   (add-hook 'font-lock-mode-hook 'my-doxymacs-font-lock-hook)
	   (my-doxymacs-stuff)
	   (setq doxymacs-doxygen-function-args "YES")
	   (setq doxymacs-namespace-doc nil))
  )


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Global hotkeys
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(global-set-key "\M-g" 'goto-line)
(global-set-key [f12] 'comment-region)
(global-set-key [f11] 'uncomment-region)
(global-set-key "\C-c\C-c" 'compile)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Startup
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(text-mode)                   ;; Set the default start up mode

(put 'scroll-left 'disabled nil)
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(package-selected-packages '(yaml-mode cpputils-cmake cmake-mode)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )
