/*
 * field_canvas.cpp
 * Canvas for the Field window
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "field_canvas.h"

#include "field_view.h"
#include "field_frame.h"
#include "confgr.h"
#include "modes.h"
#include "linmath.h"
#include "math_utils.h"
#include "background_image.h"
#include "cc_shapes.h"
#include "cc_drawcommand.h"
#include "draw.h"

#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(FieldCanvas, CtrlScrollCanvas)
EVT_CHAR(FieldCanvas::OnChar)
EVT_LEFT_DOWN(FieldCanvas::OnMouseLeftDown)
EVT_LEFT_UP(FieldCanvas::OnMouseLeftUp)
EVT_LEFT_DCLICK(FieldCanvas::OnMouseLeftDoubleClick)
EVT_RIGHT_DOWN(FieldCanvas::OnMouseRightDown)
EVT_MOTION(FieldCanvas::OnMouseMove)
EVT_PAINT(FieldCanvas::OnPaint)
EVT_ERASE_BACKGROUND(FieldCanvas::OnEraseBackground)
END_EVENT_TABLE()

// Define a constructor for field canvas
FieldCanvas::FieldCanvas(wxView *view, FieldFrame *frame, float def_zoom) :
ClickDragCtrlScrollCanvas(frame, wxID_ANY),
mFrame(frame),
mView(static_cast<FieldView*>(view)),
curr_lasso(CC_DRAG_TYPES::BOX),
curr_move(CC_MOVE_MODES::NORMAL),
drag(CC_DRAG_TYPES::NONE)
{
	SetZoom(def_zoom);
}

FieldCanvas::~FieldCanvas(void)
{
	ClearShapes();
}

// Define the repainting behaviour
void
FieldCanvas::OnPaint(wxPaintEvent& event)
{
	const auto& config = CalChartConfiguration::GetGlobalConfig();
	OnPaint(event, config);

}

void
FieldCanvas::OnPaint(wxPaintEvent& event, const CalChartConfiguration& config)
{
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);
	
	// draw the background
	PaintBackground(dc, config);
	// draw Background Image
	if (mBackgroundImage)
	{
		mBackgroundImage->OnPaint(dc);
	}
	
	// draw the view
	mView->OnDraw(&dc);
	mView->DrawOtherPoints(dc, mMovePoints);
	
	if (curr_shape)
	{
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
		CC_coord origin = mView->GetShowFieldOffset();
		for (auto i=shape_list.begin();
			 i != shape_list.end();
			 ++i)
		{
			DrawCC_DrawCommandList(dc, (*i)->GetCC_DrawCommand(origin.x, origin.y));
		}
	}
}

void
FieldCanvas::PaintBackground(wxDC& dc, const CalChartConfiguration& config)
{
	// draw the background
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetBackground(config.Get_CalChartBrushAndPen(COLOR_FIELD).first);
	dc.Clear();
}

// We have a empty erase background to improve redraw performance.
void
FieldCanvas::OnEraseBackground(wxEraseEvent& event)
{
}

// Allow clicking within pixels to close polygons
#define CLOSE_ENOUGH_TO_CLOSE 10
void
FieldCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	
	if (mBackgroundImage && mBackgroundImage->DoingPictureAdjustment())
	{
		mBackgroundImage->OnMouseLeftDown(event, dc);
	}
	else
	{
		CC_coord pos = mView->GetShowFieldOffset();
		pos.x = (x - pos.x);
		pos.y = (y - pos.y);
		
		switch (curr_move)
		{
			case CC_MOVE_MODES::LINE:
				mFrame->SnapToGrid(pos);
				BeginDrag(CC_DRAG_TYPES::LINE, pos);

				mTransformer = [this](const CC_coord&)
				{
					const CC_shape_2point *shape = (CC_shape_2point*)curr_shape.get();
					auto start = shape->GetOrigin();
					auto second = shape->GetPoint();
					CC_coord curr_pos = start;
					std::map<unsigned, CC_coord> result;
					auto& select_list = mView->GetSelectionList();
					for (auto i = select_list.begin(); i != select_list.end(); ++i, curr_pos += second - start)
					{
						result[*i] = curr_pos;
					}
					return result;
				};
				break;
			case CC_MOVE_MODES::ROTATE:
				mFrame->SnapToGrid(pos);
				if (curr_shape &&
					(((CC_shape_1point*)curr_shape.get())->GetOrigin() != pos))
				{
					AddDrag(CC_DRAG_TYPES::LINE, std::unique_ptr<CC_shape>(new CC_shape_arc(((CC_shape_1point*)
											  curr_shape.get())->GetOrigin(), pos)));

					// set up the place where points moves
					CC_coord c1 = dynamic_cast<CC_shape_1point&>(*shape_list[0]).GetOrigin();
					mTransformer = [c1, this](const CC_coord&)
					{
						auto r = -((CC_shape_arc*)curr_shape.get())->GetAngle();
						auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
							ZRotationMatrix(r) *
							TranslationMatrix(Vector(c1.x, c1.y, 0));
						return this->GetPoints(m);
					};
				}
				else
				{
					BeginDrag(CC_DRAG_TYPES::CROSS, pos);
				}
				break;
			case CC_MOVE_MODES::SHEAR:
				mFrame->SnapToGrid(pos);
				if (curr_shape &&
					(((CC_shape_1point*)curr_shape.get())->GetOrigin() != pos))
				{
					CC_coord vect(pos - ((CC_shape_1point*)curr_shape.get())->GetOrigin());
					// rotate vect 90 degrees
					AddDrag(CC_DRAG_TYPES::LINE,
							std::unique_ptr<CC_shape>(new CC_shape_angline(pos,CC_coord(-vect.y, vect.x))));

					// set up the place where points moves
					CC_coord o = dynamic_cast<CC_shape_1point&>(*shape_list[0]).GetOrigin();
					mTransformer = [o, this](const CC_coord&)
					{
						const CC_shape_2point *shape = (CC_shape_2point*)curr_shape.get();
						CC_coord c1 = shape->GetOrigin();
						CC_coord c2 = shape->GetPoint();
						CC_coord v1 = c1 - o;
						CC_coord v2 = c2 - c1;
						float amount = v2.Magnitude() / v1.Magnitude();
						if (BoundDirectionSigned(v1.Direction() - (c2-o).Direction()) < 0)
						{
							amount = -amount;
						}
						float ang = -v1.Direction()*M_PI/180.0;
						auto m = TranslationMatrix(Vector(-o.x, -o.y, 0)) *
							ZRotationMatrix(-ang) *
							YXShearMatrix(amount) *
							ZRotationMatrix(ang) *
							TranslationMatrix(Vector(o.x, o.y, 0));
						return this->GetPoints(m);
					};
				}
				else
				{
					BeginDrag(CC_DRAG_TYPES::CROSS, pos);
				}
				break;
			case CC_MOVE_MODES::REFL:
				mFrame->SnapToGrid(pos);
				BeginDrag(CC_DRAG_TYPES::LINE, pos);
				
				mTransformer = [this](const CC_coord&)
				{
					const CC_shape_2point *shape = (CC_shape_2point*)curr_shape.get();
					CC_coord c1 = shape->GetOrigin();
					CC_coord c2 = shape->GetPoint() - c1;
					float ang = -c2.Direction()*M_PI/180.0;
					auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
						ZRotationMatrix(-ang) *
						YReflectionMatrix() *
						ZRotationMatrix(ang) *
						TranslationMatrix(Vector(c1.x, c1.y, 0));
					return this->GetPoints(m);
				};
				break;
			case CC_MOVE_MODES::SIZE:
				mFrame->SnapToGrid(pos);
				if (curr_shape &&
					(((CC_shape_1point*)curr_shape.get())->GetOrigin() != pos))
				{
					AddDrag(CC_DRAG_TYPES::LINE, std::unique_ptr<CC_shape>(new CC_shape_line(pos)));
					mTransformer = [this](const CC_coord&) -> std::map<unsigned, CC_coord>
					{
						auto& origin = dynamic_cast<CC_shape_1point&>(*shape_list[0]);
						CC_coord c1 = origin.GetOrigin();
						const CC_shape_2point *shape = (CC_shape_2point*)curr_shape.get();
						CC_coord c2 = shape->GetPoint() - c1;
						float sx = c2.x;
						float sy = c2.y;
						c2 = shape->GetOrigin() - c1;
						if ((c2.x != 0) || (c2.y != 0))
						{
							if (c2.x != 0)
							{
								sx /= c2.x;
							}
							else
							{
								sx = 1;
							}
							if (c2.y != 0)
							{
								sy /= c2.y;
							}
							else
							{
								sy = 1;
							}
							auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
								ScaleMatrix(Vector(sx, sy, 0)) *
								TranslationMatrix(Vector(c1.x, c1.y, 0));
							return this->GetPoints(m);
						}
						return std::map<unsigned, CC_coord>{};
					};
					
				}
				else
				{
					BeginDrag(CC_DRAG_TYPES::CROSS, pos);
				}
				break;
			case CC_MOVE_MODES::GENIUS:
				mFrame->SnapToGrid(pos);
				AddDrag(CC_DRAG_TYPES::LINE, std::unique_ptr<CC_shape>(new CC_shape_line(pos)));
				if (shape_list.size() > 2)
				{
					mTransformer = [this](const CC_coord&) -> std::map<unsigned, CC_coord>
					{
						CC_shape_2point* v1 = (CC_shape_2point*)shape_list[0].get();
						CC_shape_2point* v2 = (CC_shape_2point*)shape_list[1].get();
						CC_shape_2point* v3 = (CC_shape_2point*)shape_list[2].get();
						
						CC_coord s1 = v1->GetOrigin();
						CC_coord e1 = v1->GetPoint();
						CC_coord s2 = v2->GetOrigin();
						CC_coord e2 = v2->GetPoint();
						CC_coord s3 = v3->GetOrigin();
						CC_coord e3 = v3->GetPoint();
						auto d = (float)s1.x*(float)s2.y - (float)s2.x*(float)s1.y + (float)s3.x*(float)s1.y - (float)s1.x*(float)s3.y + (float)s2.x*(float)s3.y - (float)s3.x*(float)s2.y;
						if (IS_ZERO(d))
						{
							return std::map<unsigned, CC_coord>{};
						}
						else
						{
							Matrix A = Matrix(Vector(e1.x,e2.x,0,e3.x),
											  Vector(e1.y,e2.y,0,e3.y),
											  Vector(0,0,0,0),
											  Vector(1,1,0,1));
							Matrix Binv = Matrix(Vector((float)s2.y-(float)s3.y,
														(float)s3.x-(float)s2.x, 0,
														(float)s2.x*(float)s3.y -
														(float)s3.x*(float)s2.y),
												 Vector((float)s3.y-(float)s1.y,
														(float)s1.x-(float)s3.x, 0,
														(float)s3.x*(float)s1.y -
														(float)s1.x*(float)s3.y),
												 Vector(0, 0, 0, 0),
												 Vector((float)s1.y-(float)s2.y,
														(float)s2.x-(float)s1.x, 0,
														(float)s1.x*(float)s2.y -
														(float)s2.x*(float)s1.y));
							Binv /= d;
							Matrix m = Binv*A;
							return this->GetPoints(m);
						}
					};
				}
				break;
			case CC_MOVE_MODES::SWAP:
			{
				int targetDotIndex = mView->FindPoint(pos);
				if (targetDotIndex >= 0) {
					SelectionList targetDot;
					targetDot.insert(targetDotIndex);
					if (mView->GetSelectionList().size() != 1) {
						mView->UnselectAll();
					}
					mView->AddToSelection(targetDot);
					if (mView->GetSelectionList().size() == 2) {
						mView->DoRotatePointPositions(1);
						mView->UnselectAll();
					}
				} else {
					mView->UnselectAll();
				}
				break;
			}
			default:
				switch (drag)
			{
				case CC_DRAG_TYPES::POLY:
				{
					auto* p = ((CC_lasso*)curr_shape.get())->FirstPoint();
					float d;
					if (p != NULL)
					{
						// need to know where the scale is, so we need the device.
						wxClientDC dc(this);
						PrepareDC(dc);
						Coord polydist =
						dc.DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
						d = p->x - pos.x;
						if (std::abs(d) < polydist)
						{
							d = p->y - pos.y;
							if (std::abs(d) < polydist)
							{
								mView->SelectWithLasso((CC_lasso*)curr_shape.get(), event.AltDown());
								EndDrag();
								break;
							}
						}
					}
					((CC_lasso*)curr_shape.get())->Append(pos);
				}
					break;
				default:
					if (!(event.ShiftDown() || event.AltDown()))
					{
						mView->UnselectAll();
					}
					int i = mView->FindPoint(pos);
					if (i < 0)
					{
						// if no point selected, we grab using the current lasso
						BeginDrag(curr_lasso, pos);
					}
					else
					{
						SelectionList select;
						select.insert(i);
						if (event.AltDown())
						{
							mView->ToggleSelection(select);
						}
						else
						{
							mView->AddToSelection(select);
						}
						
						BeginDrag(CC_DRAG_TYPES::LINE, mView->PointPosition(i));
						mTransformer = [this](const CC_coord&)
						{
							const CC_shape_2point *shape = (CC_shape_2point*)curr_shape.get();
							CC_coord pos = shape->GetPoint() - shape->GetOrigin();
							std::map<unsigned, CC_coord> result;
							auto& select_list = mView->GetSelectionList();
							for (auto i = select_list.begin(); i != select_list.end(); ++i)
							{
								result[*i] = mView->PointPosition(*i) + pos;
							}
							return result;
						};
					}
			}
				break;
		}
	}
	Refresh();
}

// Allow clicking within pixels to close polygons
void
FieldCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	
	if (mBackgroundImage && mBackgroundImage->DoingPictureAdjustment())
	{
		mBackgroundImage->OnMouseLeftUp(event, dc);
	}
	else
	{
		CC_coord pos = mView->GetShowFieldOffset();
		pos.x = (x - pos.x);
		pos.y = (y - pos.y);
		
		const CC_shape_2point *shape = (CC_shape_2point*)curr_shape.get();
		if (curr_shape)
		{
			switch (curr_move)
			{
				case CC_MOVE_MODES::LINE:
					mView->DoMovePoints(mMovePoints);
					mFrame->SetCurrentMove(CC_MOVE_MODES::NORMAL);
					break;
				case CC_MOVE_MODES::ROTATE:
					if (shape_list.size() > 1)
					{
						if (shape->GetOrigin() == shape->GetPoint())
						{
							BeginDrag(CC_DRAG_TYPES::CROSS, pos);
						}
						else
						{
							mView->DoMovePoints(mMovePoints);
							mFrame->SetCurrentMove(CC_MOVE_MODES::NORMAL);
						}
					}
					break;
				case CC_MOVE_MODES::SHEAR:
					if (shape_list.size() > 1)
					{
						if (shape->GetOrigin() == shape->GetPoint())
						{
							BeginDrag(CC_DRAG_TYPES::CROSS, pos);
						}
						else
						{
							mView->DoMovePoints(mMovePoints);
							mFrame->SetCurrentMove(CC_MOVE_MODES::NORMAL);
						}
					}
					break;
				case CC_MOVE_MODES::REFL:
					if (shape->GetOrigin() != shape->GetPoint())
					{
						mView->DoMovePoints(mMovePoints);
					}
					mFrame->SetCurrentMove(CC_MOVE_MODES::NORMAL);
					break;
				case CC_MOVE_MODES::SIZE:
					if (shape_list.size() > 1)
					{
						if (shape->GetOrigin() == shape->GetPoint())
						{
							BeginDrag(CC_DRAG_TYPES::CROSS, pos);
						}
						else
						{
							mView->DoMovePoints(mMovePoints);
							mFrame->SetCurrentMove(CC_MOVE_MODES::NORMAL);
						}
					}
					break;
				case CC_MOVE_MODES::GENIUS:
					if (shape_list.size() > 2)
					{
						mView->DoMovePoints(mMovePoints);
						mFrame->SetCurrentMove(CC_MOVE_MODES::NORMAL);
					}
					break;
				default:
					switch (drag)
				{
					case CC_DRAG_TYPES::BOX:
						mView->SelectPointsInRect(shape->GetOrigin(), shape->GetPoint(), event.AltDown());
						EndDrag();
						break;
					case CC_DRAG_TYPES::LINE:
						mView->DoMovePoints(mMovePoints);
						EndDrag();
						break;
					case CC_DRAG_TYPES::LASSO:
						((CC_lasso*)curr_shape.get())->End();
						mView->SelectWithLasso((CC_lasso*)curr_shape.get(), event.AltDown());
						EndDrag();
						break;
					default:
						break;
				}
					break;
			}
		}
	}
	Refresh();
}

// Allow clicking within pixels to close polygons
void
FieldCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	
	if (curr_shape && (CC_DRAG_TYPES::POLY == drag))
	{
		mView->SelectWithLasso((CC_lasso*)curr_shape.get(), event.AltDown());
		EndDrag();
	}
	Refresh();
}

// Allow clicking within pixels to close polygons
void
FieldCanvas::OnMouseRightDown(wxMouseEvent& event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	
	if (curr_shape && (CC_DRAG_TYPES::POLY == drag))
	{
		mView->SelectWithLasso((CC_lasso*)curr_shape.get(), event.AltDown());
		EndDrag();
	}
	Refresh();
}

// Allow clicking within pixels to close polygons
void
FieldCanvas::OnMouseMove(wxMouseEvent& event)
{
	super::OnMouseMove(event);
	
	if (!IsScrolling())
	{
		wxClientDC dc(this);
		PrepareDC(dc);
		long x, y;
		event.GetPosition(&x, &y);
		x = dc.DeviceToLogicalX(x);
		y = dc.DeviceToLogicalY(y);

		if (mBackgroundImage && mBackgroundImage->DoingPictureAdjustment())
		{
			mBackgroundImage->OnMouseMove(event, dc);
		}
		else
		{
			CC_coord pos = mView->GetShowFieldOffset();
			pos.x = (x - pos.x);
			pos.y = (y - pos.y);

			if ((event.Dragging() && event.LeftIsDown() && curr_shape)
				|| (event.Moving() && curr_shape && (CC_DRAG_TYPES::POLY == drag)))
			{
				MoveDrag(pos);
			}
		}
	}
	Refresh();
}

// Intercept character input
void
FieldCanvas::OnChar(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_LEFT)
		mView->GoToPrevSheet();
	else if (event.GetKeyCode() == WXK_RIGHT)
		mView->GoToNextSheet();
	else
		event.Skip();
}

// Zoom to fit length wise, seems best idea
float
FieldCanvas::ZoomToFitFactor() const
{
	const wxSize screenSize = GetSize();
	return static_cast<float>(screenSize.GetX())/mView->GetShowFieldSize().x;
}

void
FieldCanvas::SetZoom(float factor)
{
	CtrlScrollCanvas::SetZoom(factor);
	float f = GetZoom();
	long newx = mView->GetShowFieldSize().x * f;
	long newy = mView->GetShowFieldSize().y * f;
	SetVirtualSize(newx, newy);
	Refresh();
}


std::map<unsigned, CC_coord>
FieldCanvas::GetPoints(const Matrix& transmat)
{
	std::map<unsigned, CC_coord> result;
	for (auto i : mView->GetSelectionList())
	{
		auto c = mView->PointPosition(i);
		Vector v(c.x, c.y, 0);
		v = transmat * v;
		v.Homogenize();
		c = CC_coord(RoundToCoord(v.GetX()), RoundToCoord(v.GetY()));
		result[i] = c;
	}
	return result;
}

void
FieldCanvas::BeginDrag(CC_DRAG_TYPES type, const CC_coord& start)
{
	drag = type;
	ClearShapes();
	curr_shape.reset();
	switch (type)
	{
		case CC_DRAG_TYPES::BOX:
			AddDrag(type, std::unique_ptr<CC_shape>(new CC_shape_rect(start)));
			break;
		case CC_DRAG_TYPES::POLY:
			AddDrag(type, std::unique_ptr<CC_shape>(new CC_poly(start)));
			break;
		case CC_DRAG_TYPES::LASSO:
			AddDrag(type, std::unique_ptr<CC_shape>(new CC_lasso(start)));
			break;
		case CC_DRAG_TYPES::LINE:
			AddDrag(type, std::unique_ptr<CC_shape>(new CC_shape_line(start)));
			break;
		case CC_DRAG_TYPES::CROSS:
			AddDrag(type, std::unique_ptr<CC_shape>(new CC_shape_cross(start, Int2Coord(2))));
		default:
			break;
	}
	std::map<unsigned, CC_coord> mPositions;

}

void
FieldCanvas::AddDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape)
{
	drag = type;
	// convert shape to shared_ptr
	std::shared_ptr<CC_shape> shared_shape(std::move(shape));
	shape_list.push_back(shared_shape);
	curr_shape = shared_shape;
}

void
FieldCanvas::MoveDrag(const CC_coord& end)
{
	if (curr_shape)
	{
		CC_coord snapped = end;
		mFrame->SnapToGrid(snapped);
		curr_shape->OnMove(end, snapped);
	}
	if (mTransformer)
	{
		mMovePoints = mTransformer(end);
	}
}

void
FieldCanvas::EndDrag()
{
	mMovePoints.clear();
	ClearShapes();
	mTransformer = nullptr;
	drag = CC_DRAG_TYPES::NONE;
}

bool
FieldCanvas::SetBackgroundImage(const wxImage& image)
{
	if (!image.IsOk())
	{
		return false;
	}
	
	wxClientDC dc(this);
	PrepareDC(dc);
	long x = 100;
	long y = 100;
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	
	mBackgroundImage.reset(new BackgroundImage(image, x, y));
	Refresh();
	return true;
}

void
FieldCanvas::AdjustBackgroundImage(bool enable)
{
	if (mBackgroundImage)
		mBackgroundImage->DoPictureAdjustment(enable);
	Refresh();
}

void
FieldCanvas::RemoveBackgroundImage()
{
	mBackgroundImage.reset();
	Refresh();
}

CC_DRAG_TYPES
FieldCanvas::GetCurrentLasso() const
{
	return curr_lasso;
}

void
FieldCanvas::SetCurrentLasso(CC_DRAG_TYPES lasso)
{
	curr_lasso = lasso;
}

CC_MOVE_MODES
FieldCanvas::GetCurrentMove() const
{
	return curr_move;
}

// implies a call to EndDrag()
void
FieldCanvas::SetCurrentMove(CC_MOVE_MODES move)
{
	EndDrag();
	curr_move = move;
}

void
FieldCanvas::ClearShapes()
{
	shape_list.clear();
	curr_shape.reset();
}

