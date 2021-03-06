#include "eosDock.h"
#include "eosDockIcon.h"
#include "axLib/axWindowManager.h"
#include "eosSystemProxy.h"
#include "eosSystemAppManager.h"

const int eos::Dock::ICON_Y_POSITION = 5;

eos::Dock::Dock(const ax::Rect& rect)
	: _anim_percent(1.0)
	, _anim_active(false)
	, _timer_interval(20)
	, _timer_length(100)
	, _anim_up(true)
	, _up_rect(rect)
	, _drop_rect(ax::Rect(rect.position.x, rect.position.y + rect.size.y - 10, rect.size.x, 10))
{
	ax::App& app = ax::App::GetInstance();
	_timer_up = new ax::Event::Timer(app.GetEventManager(), GetOnAnimationTimerUp());
	_timer_down = new ax::Event::Timer(app.GetEventManager(), GetOnAnimationTimerDown());

	win = ax::Window::Create(rect);
	win->event.OnPaint = ax::WBind<ax::GC>(this, &Dock::OnPaint);
	win->event.OnMouseEnter = ax::WBind<ax::Point>(this, &Dock::OnMouseEnter);
	win->event.OnMouseLeave = ax::WBind<ax::Point>(this, &Dock::OnMouseLeave);

//	win->event.OnBeforeDrawing = ax::WFunc<int>([&](const int& nothing) {
//		if (win->state.Get(ax::Window::StateOption::NeedUpdate)) {
//			unsigned char* bg_data = nullptr;
//			ax::Rect d_rect = win->GetWindowPixelData(bg_data);
//			_bg_img = ax::Image::Ptr(new ax::Image(bg_data, d_rect.size, ax::Image::ColorType::RGBA));
//			delete bg_data;
//		}
//	});

	/// @todo Change event id.
	win->AddConnection(320, GetOnAppSelect());

	ax::Size icon_size(64, 64);

	ax::StringPairVector apps_icon_info = { ax::StringPair("resources/1441952929_house.png", "browser"),
		ax::StringPair("resources/1441951759_calculator.png", "calc"),
		ax::StringPair("resources/1441953077_notepad.png", "TextEditor"),
		ax::StringPair("resources/1441952725_terminal.png", "Terminal"),
		ax::StringPair("resources/1441953272_enveloppe-alt.png", "mail"),
		ax::StringPair("resources/1441952856_calendar.png", "calender"),
		ax::StringPair("resources/1441952883_book.png", "book"),
		ax::StringPair("resources/1441953050_image.png", "viewer"),
		ax::StringPair("resources/1441953912_wrench-screwdriver.png", "settings"),
		ax::StringPair("resources/1441954538_appicns_Trash_Empty.png", "app_test") };

	ax::Point icon_pos(10, ICON_Y_POSITION);

	for (auto& n : apps_icon_info) {

		ax::Window::Ptr icon = win->node.Add(std::shared_ptr<eos::DockIcon>(
			new eos::DockIcon(ax::Rect(icon_pos, icon_size), n.first, n.second)));

		_app_icons.push_back(static_cast<eos::DockIcon*>(icon->backbone.get()));

		icon_pos = icon->dimension.GetRect().GetNextPosRight(10);
	}

//	_appLoaders["calc"] = AppLoader("./app/calculator.so");
//	_appLoaders["browser"] = AppLoader("./app/browser.so");
//	_appLoaders["TextEditor"] = AppLoader("./app/text_editor.so");
//	_appLoaders["term"] = AppLoader("./app/terminal.so");
//	_appLoaders["viewer"] = AppLoader("./app/video.so");
//	_appLoaders["mail"] = AppLoader("./app/mail.so");
//	_appLoaders["calender"] = AppLoader("./app/slideshow.so");
//	_appLoaders["book"] = AppLoader("./app/editor.so");
//	_appLoaders["settings"] = AppLoader("./app/mail.so");
//	_appLoaders["app_test"] = AppLoader("./app/mail.so");

	win->dimension.SetRect(_drop_rect);
	_isDrop = true;

	for (auto& n : _app_icons) {
		n->GetWindow()->Hide();
	}

	eos::sys::proxy::ConnectToAppManager(eos::sys::AppManager::FRAME_FULL_SCREEN, GetOnWindowFullScreen());
}

void eos::Dock::OnAnimationTimerUp(const ax::Event::Timer::Msg& msg)
{
	if (_anim_up == true) {
		int count = (int)msg.GetTime().count() + _timer_interval;
		_anim_percent = float(count) / float(_timer_length);


//		int dock_y_pos = 10 + _anim_percent * (_up_rect.size.y - 10);
		int dock_y_size = 10 + _anim_percent * (_up_rect.size.y - 10);
		int dock_y_pos = _up_rect.size.y - dock_y_size;
		int icon_y_pos = dock_y_pos - ICON_Y_POSITION;
		
		// Set icons alpha.
		for (auto& n : _app_icons) {
			n->SetAlpha(_anim_percent);
			ax::Point icon_pos(n->GetWindow()->dimension.GetRect().position);
			n->GetWindow()->dimension.SetPosition(ax::Point(icon_pos.x, icon_y_pos));
			
		}

		// When done going up.
		if (count == _timer_length) {
			_anim_percent = 1.0;
			_anim_active = false;

			// Set icons alpha to 1.
			for (auto& n : _app_icons) {
				n->SetAlpha(_anim_percent);
				ax::Point icon_pos(n->GetWindow()->dimension.GetRect().position);
				n->GetWindow()->dimension.SetPosition(ax::Point(icon_pos.x, ICON_Y_POSITION));
			}
		}

		win->Update();
	}
}

void eos::Dock::OnAnimationTimerDown(const ax::Event::Timer::Msg& msg)
{
	if (_anim_up == false) {
		int count = (int)msg.GetTime().count() + _timer_interval;
		_anim_percent = float(count) / float(_timer_length);

		int dock_y_pos = 10 + _anim_percent * (_up_rect.size.y - 10);
//		int dock_y_pos = _up_rect.size.y - dock_y_size;
		int icon_y_pos = dock_y_pos - ICON_Y_POSITION;
		
		for (auto& n : _app_icons) {
			n->SetAlpha(1.0 - _anim_percent);
			ax::Point icon_pos(n->GetWindow()->dimension.GetRect().position);
			n->GetWindow()->dimension.SetPosition(ax::Point(icon_pos.x, icon_y_pos));
		}

		// When done going down.
		if (count == _timer_length) {
			_anim_percent = 1.0;
			_anim_active = false;

			win->dimension.SetRect(_drop_rect);

			// Hide all icons.
			for (auto& n : _app_icons) {
				n->GetWindow()->Hide();
			}
		}

		win->Update();
	}
}

bool eos::Dock::IsDrop() const
{
	return _isDrop;
}

void eos::Dock::OnMouseEnter(const ax::Point& mouse)
{
	if (_isDrop) {
		ax::Print("Mouse enter.");
		win->dimension.SetRect(_up_rect);

		_isDrop = false;
		_anim_active = true;
		_anim_percent = 0.0;
		_anim_up = true;

		if (_timer_down->IsRunning()) {
			_timer_down->StopTimer();
		}

		_timer_up->StartTimer(ax::Event::Timer::TimeMs(_timer_interval), // Interval.
			ax::Event::Timer::TimeMs(_timer_length)); // Length.

		// Show all icon on dock.
		for (auto& n : _app_icons) {
			n->GetWindow()->Show();
		}
	}
}

void eos::Dock::OnMouseLeave(const ax::Point& mouse)
{
	bool over = win->dimension.GetAbsoluteRect().IsPointInside(mouse);

	if (over == false) {
		_isDrop = true;
		_anim_active = true;
		_anim_percent = 0.0;
		_anim_up = false;

		if (_timer_up->IsRunning()) {
			_timer_up->StopTimer();
		}

		_timer_down->StartTimer(
			ax::Event::Timer::TimeMs(_timer_interval), ax::Event::Timer::TimeMs(_timer_length));

		// Hide all icons on dock.
		//		for (auto& n : _app_icons) {
		//			n->GetWindow()->Hide();
		//		}
	}
}

void eos::Dock::OnAppSelect(const ax::Event::StringMsg& msg)
{
	eos::sys::proxy::LaunchApplication(msg.GetMsg());
}

void eos::Dock::OnWindowMinimize(const eos::Frame::Msg& msg)
{
	ax::Print("Window minimize.");
	eos::Frame* frame = msg.GetSender();

	if (frame != nullptr) {
		frame->GetWindow()->Hide();
	}
}

void eos::Dock::OnWindowFullScreen(const eos::Frame::Msg& msg)
{
	std::string app_name(msg.GetSender()->GetAppName());

	// Find app icon and add fullscreen button.
	for (auto& n : _app_icons) {
		if (n->GetName() == app_name) {
			n->ActivateFullScreenBtn();
			break;
		}
	}
}

void eos::Dock::OnWindowClose(const eos::Frame::Msg& msg)
{
	// ax::Print("Window close.");
	//	eos::Frame* frame = msg.GetSender();
	//
	//	if(frame != nullptr)
	//	{
	//		for(auto& n : _appLoaders)
	//		{
	//			ax::Window* handle = n.second.GetHandle();
	//
	//			if(handle != nullptr)
	//			{
	//				if(handle->GetId() == frame->GetId())
	//				{
	//					//ax::Print("Before remove handle.");
	//					_appLoaders[n.first].RemoveHandle();
	//					frame->DeleteWindow();
	//					//ax::Print("Delete before return.");
	//					return;
	//				}
	//			}
	//		}
	//	}
}

void DrawRoundedRectangle(
	ax::GC gc, const ax::Rect& rect, const int& radius, const ax::Color& c1, const ax::Color& c2)
{
	int r = radius;

	if (r > rect.size.y * 0.5) {
		r = rect.size.y * 0.5;
	}

	const ax::FloatRect frect(rect.Cast<float>());
	const ax::FloatPoint& frp = frect.position;
	const ax::FloatPoint& frs = frect.size;

	// Middle rentangle.
	{
		const ax::Rect center_rect(frp.x + r - 1, frp.y - 1, frs.x - 2.0 * r + 1, frs.y + 1);
		gc.DrawRectangleVerticalColorFade(center_rect, c2, c1);
	}

	const int size_rect_height = frs.y - 1.0 * r + 1;

	// Left and right rectangles.
	const ax::FloatRect lrect(frp.x - 1, frp.y + r - 1, r, size_rect_height);
	const ax::FloatRect rrect(frp.x + frect.size.x - r, frp.y + r - 1, r + 1, size_rect_height);

	const double color_ratio = 1.0 - lrect.size.y / frs.y;
	const ax::Color lc1(c1.Interpolate(c2, color_ratio));

	// Draw side rectangles.
	int length = lrect.size.y + 1;
	std::vector<ax::Point> points_left, points_right;
	std::vector<ax::Color> colors_left, colors_right;

	points_left.reserve(length);
	points_right.reserve(length);
	colors_left.reserve(length);
	colors_right.reserve(length);

	const float inv_rect_y = 1.0 / float(rect.size.y);

	for (int y = lrect.position.y + 1; y <= lrect.position.y + lrect.size.y; y++) {
		const float cc_ratio = y * inv_rect_y;
		const ax::Color cc(c1.Interpolate(c2, cc_ratio));

		// Left rectangle.
		points_left.push_back(ax::Point(lrect.position.x, y));
		points_left.push_back(ax::Point(lrect.position.x + lrect.size.x, y));

		colors_left.push_back(c1);
		colors_left.push_back(cc);

		// Right rectangle.
		points_right.push_back(ax::Point(rrect.position.x, y));
		points_right.push_back(ax::Point(rrect.position.x + rrect.size.x, y));

		colors_right.push_back(cc);
		colors_right.push_back(c1);
	}

	gc.DrawLines(points_left, colors_left);
	gc.DrawLines(points_right, colors_right);

	// Top left corner.
	gc.DrawQuarterCircle(ax::FloatPoint(frp.x + r - 1, frp.y + r - 1), r, M_PI, 20, lc1, c1);

	// Top right corner.
	gc.DrawQuarterCircle(ax::FloatPoint(frp.x + frs.x - r, frp.y + r - 1), r, 3.0 * M_PI * 0.5, 20, lc1, c1);
}

void eos::Dock::OnPaint(ax::GC gc)
{
	ax::Color dock_color(0.7, 0.3);
	ax::Color dock_contour(1.0, 0.05);

	// Dock is dropped.
	if (_isDrop) {
		// Getting down.
		if (_anim_active) {
			int y_size = 10 + _anim_percent * (_up_rect.size.y - 10);
			ax::Rect dock_rect(0, y_size, _up_rect.size.x, _up_rect.size.y);

			double inv_anim = 1.0 - _anim_percent;
			ax::Color c_contour(dock_contour);
			c_contour.SetAlpha(inv_anim * c_contour.GetAlpha());

			ax::Color c_dock(dock_color);
			c_dock.SetAlpha(inv_anim * dock_color.GetAlpha());

			DrawRoundedRectangle(gc, dock_rect, 25, c_contour, c_dock);
		}
		else { // Static drop position.

			ax::Rect dock_rect(0, 0, _up_rect.size.x, _up_rect.size.y);
			DrawRoundedRectangle(gc, dock_rect, 25, dock_contour, dock_color);
		}
	}
	else {
		// Getting up.
		if (_anim_active) {

			int y_size = 10 + _anim_percent * (_up_rect.size.y - 10);
			int y_pos = _up_rect.size.y - y_size;

			ax::Rect dock_rect(0, y_pos, _up_rect.size.x, _up_rect.size.y);

			ax::Color c_contour(dock_contour);
			c_contour.SetAlpha(_anim_percent * c_contour.GetAlpha());

			ax::Color c_dock(dock_color);
			c_dock.SetAlpha(_anim_percent * dock_color.GetAlpha());

			DrawRoundedRectangle(gc, dock_rect, 25, c_contour, c_dock);
		}
		else { // Static up position.
			ax::Rect dock_rect(0, 0, _up_rect.size.x, _up_rect.size.y);
			DrawRoundedRectangle(gc, dock_rect, 25, dock_contour, dock_color);
		}
	}
}
