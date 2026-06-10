# SPDX-Lsicense-Identifier: BSD-3-Clause
# flow_bridge.py – mrv2 ↔ Autodesk Flow (ShotGrid) two-way plugin
# Uses PyFLTK + correct mrv2.plugin.Plugin API

import os
import sys
import threading
import time

# ══════════════════════════════════════════════════════════════════════════════
# CONFIGURATION
# ══════════════════════════════════════════════════════════════════════════════

# For more info, refer to this youtube video.
#
# https://www.youtube.com/watch?v=RYEBQDJiXAs&t=75s

FLOW_URL = "https://your_site.shotgrid.autodesk.com"
FLOW_SCRIPT_NAME = 'your_script name as created in Web/Script'
FLOW_API_KEY = 'Your API Key as created in Web/Scripts'

#
# The URL and API KEY below will work for 20 days.
#
FLOW_URL = "https://filmaura.shotgrid.autodesk.com"
FLOW_SCRIPT_NAME = 'my_script'
FLOW_API_KEY = 'lxhaemtakPachryqheaqxlx4&'

# ── mrv2 imports ──────────────────────────────────────────────────────────────
import mrv2
from mrv2 import cmd, media, plugin, timeline

# ── ShotGrid / Flow API ───────────────────────────────────────────────────────
try:
    import shotgun_api3
except Exception as e:
    print('shotgun_api3 raised an exception:')
    print(e)
    print("Try a manual re-Install it into mrv2's Python:\n"
          "  <mrv2-python> -m pip install shotgun_api3\n")
    pass

# ── pyFLTK API ────────────────────────────────────────────────────────────────
try:
    from fltk import *
    import fltk as Fl
except Exception as e:
    print('pyFLTK raised an exception:')
    print(e)
    print("Try a manual re-Install it into mrv2's Python:\n"
          "  <mrv2-python> -m pip install pyfltk\n")
    pass

#
# Translation engine if available
#
try:
    import gettext

    locale = cmd.rootPath() + '/python/plug-ins/locale'

    language = cmd.getLanguage()

    # Set the domain (name for your translations) and directory
    translator = gettext.translation('update-mrv2', localedir=locale,
                                     languages=[language])

    # Mark strings for translation using the _() function
    _ = translator.gettext
except Exception as e:
    print(e)
    def _gettext(text):
        return text
    _ = _gettext
    
def sg_log(x):
    print(f'        [flow] {x}')

# ══════════════════════════════════════════════════════════════════════════════
# CONNECTION MANAGER
# ══════════════════════════════════════════════════════════════════════════════
class _FlowConn:
    _sg = None

    @classmethod
    def get(cls):
        if cls._sg is None:
            sg_log(_(f'Connecting to server {FLOW_URL}'))
            sg_log(_(f'With script "{FLOW_SCRIPT_NAME}"...'))
            cls._sg = shotgun_api3.Shotgun(
                FLOW_URL,
                script_name=FLOW_SCRIPT_NAME,
                api_key=FLOW_API_KEY,
            )
            if cls._sg:
                sg_log(_(f'Connected.'))
            else:
                sg_log(_(f'Could not connect...'))
        return cls._sg

    @classmethod
    def reset(cls):
        cls._sg = None

def sg():
    return _FlowConn.get()


# ══════════════════════════════════════════════════════════════════════════════
# DATA LAYER
# ══════════════════════════════════════════════════════════════════════════════

def fetch_projects():
    return sg().find(
        "Project",
        [["sg_status", "is", "Active"]],
        ["id", "name", "sg_status"],
    )


def fetch_shots(project_id: int):
    return sg().find(
        "Shot",
        [["project", "is", {"type": "Project", "id": project_id}]],
        ["id", "code", "sg_status_list", "description"],
        order=[{"field_name": "code", "direction": "asc"}],
    )


def fetch_versions(shot_id: int):
    return sg().find(
        "Version",
        [["entity", "is", {"type": "Shot", "id": shot_id}]],
        ["id", "code", "sg_path_to_movie", "sg_status_list",
         "created_at", "user", "description"],
        order=[{"field_name": "created_at", "direction": "desc"}],
    )


# ══════════════════════════════════════════════════════════════════════════════
# SUBMIT HELPER
# ══════════════════════════════════════════════════════════════════════════════

def submit_version(
    project_id: int,
    shot_id: int,
    movie_path: str,
    version_name: str = "",
    description: str = "",
    update_shot_status: str = "",
    upload_media = False
) -> dict:
    if not os.path.isfile(movie_path):
        raise FileNotFoundError(f"Movie file not found: {movie_path}")

    if not version_name:
        version_name = os.path.splitext(os.path.basename(movie_path))[0]

    version_data = {
        "project":          {"type": "Project", "id": project_id},
        "entity":           {"type": "Shot",    "id": shot_id},
        "code":             version_name,
        "description":      description,
        "sg_path_to_movie": movie_path,
        "sg_status_list":   "rev",
    }

    version = sg().create("Version", version_data)
    sg_log(_(f"Created Version '{version['code']}' (id={version['id']})"))

    if upload_media:
        sg().upload("Version", version["id"], movie_path, field_name="sg_uploaded_movie")
        sg_log(_(f"Media uploaded."))

    if update_shot_status:
        sg().update("Shot", shot_id, {"sg_status_list": update_shot_status})
        sg_log(_(f"Shot status → {update_shot_status}"))

    Fl.check()

    return version


# ══════════════════════════════════════════════════════════════════════════════
# mrv2 HELPERS
# ══════════════════════════════════════════════════════════════════════════════

def _open_in_mrv2(path: str):
    """Open media in mrv2."""
    try:
        cmd.open(path)
    except Exception as e:
        print(_(f"Could not open {path}: {e}"))


def _current_media_path() -> str:
    """Return full path of the current A file (most common use-case)."""
    try:
        files = media.activeFiles()
        if files and len(files) > 0:
            # FileMedia has .path which is a mrv2.Path object
            p = files[0].path
            if hasattr(p, 'get'):
                return p.get()
            return str(p)
    except Exception:
        pass
    return ""


# ══════════════════════════════════════════════════════════════════════════════
# FLTK BROWSER PANEL
# ══════════════════════════════════════════════════════════════════════════════

class FlowBrowserPanel:
    def __init__(self):
        self.win = None
        self._projects = []
        self._shots = {}
        self._versions = {}

        self._active_project = None
        self._active_shot = None
        
        self._active_project_idx = 0
        self._active_shot_idx = 0
        self._active_version_idx = 0

        self._build_ui()
        self._load_projects()

    def _refresh_cb(self, widget):
        # First refresh projects
        self._load_projects()

        # Now refresh shot list
        self._shot_list.value(self._active_shot_idx)
        self._on_shot_change(widget)

        # Finally refresh version list
        self._version_list.value(self._active_version_idx)
        self._on_version_change(widget)
        
    def _build_ui(self):
        self.win = Fl_Double_Window(820, 620, _("Flow Browser – mrv2"))
        self.win.begin()

        # Top bar
        bar = Fl_Group(10, 10, self.win.w() - 20, 40)
        bar.begin()

        Fl_Box(10, 15, 60, 25, _("Project:"))
        self._project_cb = Fl_Choice(75, 15, 280, 25)
        self._project_cb.callback(self._on_project_change)

        
        refresh_btn = Fl_Button(self.win.w() - 140, 15, 120, 25, _("↻ Refresh"))
        refresh_btn.callback(self._refresh_cb)
        bar.end()

        # Main panes
        y = 60
        h = self.win.h() - y - 80

        # Shots
        self._shot_frame = Fl_Group(10, y, 240, h, _("Shots"))
        self._shot_frame.box(FL_ENGRAVED_FRAME)
        self._shot_list = Fl_Hold_Browser(15, y + 25, 230, h - 35)
        self._shot_list.textcolor(FL_BLACK)
        self._shot_list.selection_color(FL_CYAN)
        self._shot_list.callback(self._on_shot_change)
        self._shot_frame.end()

        # Versions
        self._ver_frame = Fl_Group(260, y, 240, h, _("Versions"))
        self._ver_frame.box(FL_ENGRAVED_FRAME)
        self._version_list = Fl_Hold_Browser(265, y + 25, 230, h - 35)
        self._version_list.textcolor(FL_BLACK)
        self._version_list.selection_color(FL_CYAN)
        self._version_list.callback(self._on_version_change)
        self._ver_frame.end()

        # Details
        self._det_frame = Fl_Group(510, y, 300, h, _("Details"))
        self._det_frame.box(FL_ENGRAVED_FRAME)
        self._detail = Fl_Text_Display(520, y + 25, 280, h - 35)
        self._detail.textcolor(FL_BLACK)
        self._detail.selection_color(FL_CYAN)
        self._detail.buffer(Fl_Text_Buffer())
        self._detail.wrap_mode(Fl_Text_Display.WRAP_AT_BOUNDS, 300)
        self._det_frame.end()

        # Bottom bar
        btn_y = self.win.h() - 50
        open_btn = Fl_Button(10, btn_y, 180, 35, _("▶  Open in mrv2"))
        open_btn.callback(self._open_selected)

        submit_btn = Fl_Button(200, btn_y, 200, 35, _("⬆  Submit current clip"))
        submit_btn.callback(self._open_submit_dialog)

        close_btn = Fl_Button(self.win.w() - 110, btn_y, 100, 35, _("Close"))
        close_btn.callback(lambda w: self.win.hide())

        self.win.end()
        self.win.show()

    # ── Callbacks ─────────────────────────────────────────────────────────────

    def _load_projects_cb(self, widget):
        self._load_projects()

    def _load_projects(self):
        try:
            self._projects = fetch_projects()
            self._project_cb.clear()
            for p in self._projects:
                self._project_cb.add(p["name"])
            if self._projects:
                self._project_cb.value(self._active_project_idx)
                self._on_project_change(None)
        except Exception as e:
            Fl.fl_alert(_(f"Connection error:\n{str(e)}"))

    def _on_project_change(self, widget):
        idx = self._project_cb.value()

        if idx < 0 or idx >= len(self._projects):
            return
        
        proj = self._projects[idx]
        self._active_project = proj
        self._active_project_idx = idx

        self._shot_list.clear()
        self._version_list.clear()
        self._clear_details()

        try:
            shots = fetch_shots(proj["id"])
            self._shots = {s["code"]: s for s in shots}
            for s in shots:
                status = s.get("sg_status_list", "—")
                self._shot_list.add(f"{s['code']}  [{status}]")
                description = s.get("description", "")
                self._detail.buffer().text(description)
        except Exception as e:
            Fl.fl_alert(str(e))

    def _on_shot_change(self, widget):
        idx = self._shot_list.value()
        if idx <= 0:
            return
        line = self._shot_list.text(idx)
        code = line.split("  ")[0].strip()
        shot = self._shots.get(code)
        if not shot:
            return
        self._active_shot = shot
        self._active_shot_idx = idx

        self._version_list.clear()
        self._clear_details()

        try:
            versions = fetch_versions(shot["id"])
            self._versions = {v["code"]: v for v in versions}
            for v in versions:
                self._version_list.add(v["code"])
        except Exception as e:
            Fl.fl_alert(str(e))

    def _on_version_change(self, widget):
        idx = self._version_list.value()
        if idx <= 0:
            return
        code = self._version_list.text(idx)
        v = self._versions.get(code)
        if not v:
            return

        self._active_version_idx = idx

        artist = (v.get("user") or {}).get("name", "—")
        info = (
            f"Code:     {v.get('code', '—')}\n"
            f"Status:   {v.get('sg_status_list', '—')}\n"
            f"Artist:   {artist}\n"
            f"Created:  {v.get('created_at', '—')}\n\n"
            f"Path:\n{v.get('sg_path_to_movie', '—')}\n\n"
            f"Notes:\n{v.get('description') or '—'}"
        )
        buf = self._detail.buffer()
        buf.text(info)

    def _clear_details(self):
        if self._detail.buffer():
            self._detail.buffer().text("")

    def _open_selected(self, widget):
        sel = self._version_list.value()
        if sel <= 0:
            Fl.fl_alert("Select a version first.")
            return
        code = self._version_list.text(sel)
        v = self._versions.get(code)
        path = (v or {}).get("sg_path_to_movie")
        if path and os.path.isfile(path):
            _open_in_mrv2(path)
        else:
            Fl.fl_alert(f"File not found:\n{path}")

    def _open_submit_dialog(self, widget):
        current = _current_media_path()
        SubmitDialog(
            parent=self.win,
            default_path=current,
            projects=self._projects,
            shots=self._shots,
            active_project=self._active_project,
            active_shot=self._active_shot,
            on_submit=self._do_submit,
        )

    def _do_submit(self, project_id, shot_id, movie_path, version_name, description, shot_status, upload_media):
        try:
            v = submit_version(
                project_id, shot_id, movie_path,
                version_name, description, shot_status or "",
                upload_media
            )
            self._refresh_cb(None)
        except Exception as e:
            print(f"Submit failed:\n{str(e)}", None)


# ══════════════════════════════════════════════════════════════════════════════
# SUBMIT DIALOG (FLTK)
# ══════════════════════════════════════════════════════════════════════════════

class SubmitDialog:
    
    def __init__(self, parent, default_path, projects, shots,
                 active_project, active_shot, on_submit):
        self._on_submit = on_submit
        self._projects = projects
        self._shots = shots

        self._read_prefs()
        
        Fl_Group.current(None)
        self.win = Fl_Double_Window(520, 420, _("Submit to Flow"))
        self.win.begin()

        y = 20
        
        # Path
        Fl_Box(20, y, 120, 25, _("Movie path:"))
        self._path_input = Fl_Input(150, y, 340, 25)
        self._path_input.value(default_path)
        self._path_input.textcolor(FL_BLACK)
        y += 40

        # Project
        Fl_Box(20, y, 120, 25, _("Project:"))
        self._proj_choice = Fl_Choice(150, y, 340, 25)
        for p in projects:
            self._proj_choice.add(p["name"])
        if active_project:
            # Find index
            for i, p in enumerate(projects):
                if p["name"] == active_project.get("name"):
                    self._proj_choice.value(i)
                    break
        self._proj_choice.callback(self._on_project_change)
        y += 40

        # Shot
        Fl_Box(20, y, 120, 25, _("Shot:"))
        self._shot_choice = Fl_Choice(150, y, 340, 25)
        self._populate_shots()
        if active_shot:
            for i in range(self._shot_choice.size()):
                if active_shot["code"] in self._shot_choice.text(i):
                    self._shot_choice.value(i)
                    break
        y += 40

        # Version name
        Fl_Box(20, y, 120, 25, _("Version name:"))
        self._name_input = Fl_Input(150, y, 340, 25)
        self._name_input.textcolor(FL_BLACK)
        y += 40

        # Description
        Fl_Box(20, y, 120, 25, _("Notes:"))
        self._desc_input = Fl_Multiline_Input(150, y, 340, 80)
        self._desc_input.textcolor(FL_BLACK)
        y += 95

        # Status
        Fl_Box(20, y, 120, 25, _("Shot status:"))
        self._status_choice = Fl_Choice(150, y, 120, 25)
        for s in ["", "wtg", "ip", "fin"]:
            self._status_choice.add(s)
        self._status_choice.value(0)  # ""
        y += 40
        
        # Upload Media
        Fl_Box(20, y, 120, 25, _("Upload Media:"))
        self._upload_choice = Fl_Choice(150, y, 120, 25)
        for s in [_("No"), _("Yes")]:
            self._upload_choice.add(s)
            
        (status, idx) = self._prefs.get('upload_choice', 0)
        self._upload_choice.value(idx)
        self._upload_choice.callback(self._on_upload_change)
        y += 40

        # Buttons
        submit_btn = Fl_Return_Button(150, y, 140, 35, _("Submit"))
        submit_btn.callback(self._submit)

        cancel_btn = Fl_Button(310, y, 100, 35, _("Cancel"))
        cancel_btn.callback(lambda w: self.win.hide())

        self.win.end()
        self.win.set_modal()
        self.win.show()

        while self.win.visible():
            Fl.check()
            time.sleep(0.01)

        self._write_prefs()
        
        self._prefs = None
        self._app_prefs = None


    def _read_prefs(self):
        self._app_prefs = Fl_Preferences(Fl_Preferences.USER, 'filmaura', 'flow')
        
        self._prefs = Fl_Preferences(self._app_prefs, 'Flow')
        (status, self._prefs_version) = self._prefs.get('version', 0)

    def _write_prefs(self):
        self._app_prefs.flush()
        
    def _on_upload_change(self, widget):
        idx = widget.value()
        self._prefs.set('upload_choice', idx)
        self._write_prefs()
            
    def _populate_shots(self):
        self._shot_choice.clear()
        pid = self._get_selected_project_id()
        if pid:
            shots = fetch_shots(pid)
            self._shots = {s["code"]: s for s in shots}
            for s in shots:
                self._shot_choice.add(s["code"])

    def _on_project_change(self, widget):
        self._populate_shots()

    def _get_selected_project_id(self):
        idx = self._proj_choice.value()
        if 0 <= idx < len(self._projects):
            return self._projects[idx]["id"]
        return None

    def _submit(self, widget):
        self.win.hide()
        
        path = self._path_input.value().strip()
        if not path or not os.path.isfile(path):
            Fl.fl_alert(_("Valid movie path required."))
            return

        proj_id = self._get_selected_project_id()
        shot_idx = self._shot_choice.value()
        if shot_idx < 0:
            Fl.fl_alert(_("Select a shot."))
            return
        shot_code = self._shot_choice.text(shot_idx)
        shot_id = self._shots.get(shot_code, {}).get("id")

        if not proj_id or not shot_id:
            Fl.fl_alert(_("Invalid project/shot selection."))
            return

        version_name = self._name_input.value().strip()
        description = self._desc_input.value()
        status = self._status_choice.text(self._status_choice.value())
        upload_media = bool(self._upload_choice.value())

        self._on_submit(proj_id, shot_id, path, version_name, description,
                        status, upload_media)


# ══════════════════════════════════════════════════════════════════════════════
# PLUGIN CLASS
# ══════════════════════════════════════════════════════════════════════════════

class FlowBridgePlugin(mrv2.plugin.Plugin):
    def __init__(self):
        super().__init__()

    def active(self):
        return True

    def launch_browser(self):
        # Run in thread to keep mrv2 responsive
        def run_panel():
            try:
                FlowBrowserPanel()
            except Exception as e:
                print(f"[flow_bridge] Error: {e}")
        run_panel()

    def menus(self):
        menus = {
            _("Flow/Browse and Submit"): self.launch_browser,
        }
        return menus


# Instantiate the plugin
flow_plugin = FlowBridgePlugin()
