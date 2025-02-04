/*************************************************************************/
/*  body_sw.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "body_sw.h"
#include "space_sw.h"
#include "area_sw.h"

void BodySW::_update_inertia() {

	if (get_space() && !inertia_update_list.in_list())
		get_space()->body_add_to_inertia_update_list(&inertia_update_list);

}


void BodySW::_update_inertia_tensor() {

	Basis tb = get_transform().basis;
	tb.scale(_inv_inertia);
	_inv_inertia_tensor = tb * get_transform().basis.transposed();

}

void BodySW::update_inertias() {

	//update shapes and motions

	switch(mode) {

		case PhysicsServer::BODY_MODE_RIGID: {

			//update tensor for allshapes, not the best way but should be somehow OK. (inspired from bullet)
			float total_area=0;

			for (int i=0;i<get_shape_count();i++) {

				total_area+=get_shape_aabb(i).get_area();
			}

			Vector3 _inertia;


			for (int i=0;i<get_shape_count();i++) {

				const ShapeSW* shape=get_shape(i);

				float area=get_shape_aabb(i).get_area();

				float mass = area * this->mass / total_area;

				_inertia += shape->get_moment_of_inertia(mass) + mass * get_shape_transform(i).get_origin();

			}

			if (_inertia!=Vector3())
				_inv_inertia=_inertia.inverse();
			else
				_inv_inertia=Vector3();

			if (mass)
				_inv_mass=1.0/mass;
			else
				_inv_mass=0;

		} break;

		case PhysicsServer::BODY_MODE_KINEMATIC:
		case PhysicsServer::BODY_MODE_STATIC: {

			_inv_inertia=Vector3();
			_inv_mass=0;
		} break;
		case PhysicsServer::BODY_MODE_CHARACTER: {

			_inv_inertia=Vector3();
			_inv_mass=1.0/mass;

		} break;
	}
	_update_inertia_tensor();

	//_update_shapes();

}



void BodySW::set_active(bool p_active) {

	if (active==p_active)
		return;

	active=p_active;
	if (!p_active) {
		if (get_space())
			get_space()->body_remove_from_active_list(&active_list);
	} else {
		if (mode==PhysicsServer::BODY_MODE_STATIC)
			return; //static bodies can't become active
		if (get_space())
			get_space()->body_add_to_active_list(&active_list);

		//still_time=0;
	}
/*
	if (!space)
		return;

	for(int i=0;i<get_shape_count();i++) {
		Shape3D &s=shapes[i];
		if (s.bpid>0) {
			get_space()->get_broadphase()->set_active(s.bpid,active);
		}
	}
*/
}



void BodySW::set_param(PhysicsServer::BodyParameter p_param, float p_value) {

	switch(p_param) {
		case PhysicsServer::BODY_PARAM_BOUNCE: {

			bounce=p_value;
		} break;
		case PhysicsServer::BODY_PARAM_FRICTION: {

			friction=p_value;
		} break;
		case PhysicsServer::BODY_PARAM_MASS: {
			ERR_FAIL_COND(p_value<=0);
			mass=p_value;
			_update_inertia();

		} break;
		default:{}
	}
}

float BodySW::get_param(PhysicsServer::BodyParameter p_param) const {

	switch(p_param) {
		case PhysicsServer::BODY_PARAM_BOUNCE: {

			return bounce;
		} break;
		case PhysicsServer::BODY_PARAM_FRICTION: {

			return friction;
		} break;
		case PhysicsServer::BODY_PARAM_MASS: {
			return mass;
		} break;
		default:{}
	}

	return 0;
}

void BodySW::set_mode(PhysicsServer::BodyMode p_mode) {

	PhysicsServer::BodyMode prev=mode;
	mode=p_mode;

	switch(p_mode) {
		//CLEAR UP EVERYTHING IN CASE IT NOT WORKS!
		case PhysicsServer::BODY_MODE_STATIC:
		case PhysicsServer::BODY_MODE_KINEMATIC: {

			_set_inv_transform(get_transform().affine_inverse());
			_inv_mass=0;
			_set_static(p_mode==PhysicsServer::BODY_MODE_STATIC);
			//set_active(p_mode==PhysicsServer::BODY_MODE_KINEMATIC);
			set_active(p_mode==PhysicsServer::BODY_MODE_KINEMATIC && contacts.size());
			linear_velocity=Vector3();
			angular_velocity=Vector3();
			if (mode==PhysicsServer::BODY_MODE_KINEMATIC && prev!=mode) {
				first_time_kinematic=true;
			}

		} break;
		case PhysicsServer::BODY_MODE_RIGID: {

			_inv_mass=mass>0?(1.0/mass):0;
			_set_static(false);

		} break;
		case PhysicsServer::BODY_MODE_CHARACTER: {

			_inv_mass=mass>0?(1.0/mass):0;
			_set_static(false);
		} break;
	}

	_update_inertia();
	//if (get_space())
//		_update_queries();

}
PhysicsServer::BodyMode BodySW::get_mode() const {

	return mode;
}

void BodySW::_shapes_changed() {

	_update_inertia();
}

void BodySW::set_state(PhysicsServer::BodyState p_state, const Variant& p_variant) {

	switch(p_state)	{
		case PhysicsServer::BODY_STATE_TRANSFORM: {


			if (mode==PhysicsServer::BODY_MODE_KINEMATIC) {
				new_transform=p_variant;
				//wakeup_neighbours();
				set_active(true);
				if (first_time_kinematic) {
					_set_transform(p_variant);
					_set_inv_transform(get_transform().affine_inverse());
					first_time_kinematic=false;
				}

			} else if (mode==PhysicsServer::BODY_MODE_STATIC) {
				_set_transform(p_variant);
				_set_inv_transform(get_transform().affine_inverse());
				wakeup_neighbours();
			} else {
				Transform3D t = p_variant;
				t.orthonormalize();
				new_transform=get_transform(); //used as old to compute motion
				_set_transform(t);
				_set_inv_transform(get_transform().inverse());

			}

		} break;
		case PhysicsServer::BODY_STATE_LINEAR_VELOCITY: {

			//if (mode==PhysicsServer::BODY_MODE_STATIC)
			//	break;
			linear_velocity=p_variant;
		} break;
		case PhysicsServer::BODY_STATE_ANGULAR_VELOCITY: {
			//if (mode!=PhysicsServer::BODY_MODE_RIGID)
			//	break;
			angular_velocity=p_variant;

		} break;
		case PhysicsServer::BODY_STATE_SLEEPING: {
			//?
			if (mode==PhysicsServer::BODY_MODE_STATIC || mode==PhysicsServer::BODY_MODE_KINEMATIC)
				break;
			bool do_sleep=p_variant;
			if (do_sleep) {
				linear_velocity=Vector3();
				//biased_linear_velocity=Vector3();
				angular_velocity=Vector3();
				//biased_angular_velocity=Vector3();
				set_active(false);
			} else {
				if (mode!=PhysicsServer::BODY_MODE_STATIC)
					set_active(true);
			}
		} break;
		case PhysicsServer::BODY_STATE_CAN_SLEEP: {
			can_sleep=p_variant;
			if (mode==PhysicsServer::BODY_MODE_RIGID && !active && !can_sleep)
				set_active(true);

		} break;
	}

}
Variant BodySW::get_state(PhysicsServer::BodyState p_state) const {

	switch(p_state)	{
		case PhysicsServer::BODY_STATE_TRANSFORM: {
			return get_transform();
		} break;
		case PhysicsServer::BODY_STATE_LINEAR_VELOCITY: {
			return linear_velocity;
		} break;
		case PhysicsServer::BODY_STATE_ANGULAR_VELOCITY: {
			return angular_velocity;
		} break;
		case PhysicsServer::BODY_STATE_SLEEPING: {
			return !is_active();
		} break;
		case PhysicsServer::BODY_STATE_CAN_SLEEP: {
			return can_sleep;
		} break;
	}

	return Variant();
}


void BodySW::set_space(SpaceSW *p_space){

	if (get_space()) {

		if (inertia_update_list.in_list())
			get_space()->body_remove_from_inertia_update_list(&inertia_update_list);
		if (active_list.in_list())
			get_space()->body_remove_from_active_list(&active_list);
		if (direct_state_query_list.in_list())
			get_space()->body_remove_from_state_query_list(&direct_state_query_list);

	}

	_set_space(p_space);

	if (get_space()) {

		_update_inertia();
		if (active)
			get_space()->body_add_to_active_list(&active_list);
//		_update_queries();
		//if (is_active()) {
		//	active=false;
		//	set_active(true);
		//}

	}

}

void BodySW::_compute_area_gravity(const AreaSW *p_area) {

	if (p_area->is_gravity_point()) {

		gravity = (p_area->get_gravity_vector() - get_transform().get_origin()).normalized() * p_area->get_gravity();

	} else {
		gravity = p_area->get_gravity_vector() * p_area->get_gravity();
	}
}

void BodySW::integrate_forces(real_t p_step) {


	if (mode==PhysicsServer::BODY_MODE_STATIC)
		return;

	AreaSW *current_area = get_space()->get_default_area();
	ERR_FAIL_COND(!current_area);

	int prio = current_area->get_priority();
	int ac = areas.size();
	if (ac) {
		const AreaCMP *aa = &areas[0];
		for(int i=0;i<ac;i++) {
			if (aa[i].area->get_priority() > prio) {
				current_area=aa[i].area;
				prio=current_area->get_priority();
			}
		}
	}

	_compute_area_gravity(current_area);
	density=current_area->get_density();

	Vector3 motion;
	bool do_motion=false;

	if (mode==PhysicsServer::BODY_MODE_KINEMATIC) {

		//compute motion, angular and etc. velocities from prev transform
		linear_velocity = (new_transform.origin - get_transform().origin)/p_step;

		//compute a FAKE angular velocity, not so easy
		Basis rot=new_transform.basis.orthonormalized().transposed() * get_transform().basis.orthonormalized();
		Vector3 axis;
		float angle;

		rot.get_axis_and_angle(axis,angle);
		axis.normalize();
		angular_velocity=axis.normalized() * (angle/p_step);

		motion = new_transform.origin - get_transform().origin;
		do_motion=true;

	} else {
		if (!omit_force_integration) {
			//overriden by direct state query

			Vector3 force=gravity*mass;
			force+=applied_force;
			Vector3 torque=applied_torque;

			real_t damp = 1.0 - p_step * density;

			if (damp<0) // reached zero in the given time
				damp=0;

			real_t angular_damp = 1.0 - p_step * density * get_space()->get_body_angular_velocity_damp_ratio();

			if (angular_damp<0) // reached zero in the given time
				angular_damp=0;

			linear_velocity*=damp;
			angular_velocity*=angular_damp;

			linear_velocity+=_inv_mass * force * p_step;
			angular_velocity+=_inv_inertia_tensor.xform(torque)*p_step;
		}

		if (continuous_cd) {
			motion=linear_velocity*p_step;
			do_motion=true;
		}

	}

	applied_force=Vector3();
	applied_torque=Vector3();

	//motion=linear_velocity*p_step;

	biased_angular_velocity=Vector3();
	biased_linear_velocity=Vector3();


	if (do_motion) {//shapes temporarily extend for raycast
		_update_shapes_with_motion(motion);
	}


	current_area=NULL; // clear the area, so it is set in the next frame
	contact_count=0;

}

void BodySW::integrate_velocities(real_t p_step) {

	if (mode==PhysicsServer::BODY_MODE_STATIC)
		return;

	if (fi_callback)
		get_space()->body_add_to_state_query_list(&direct_state_query_list);

	if (mode==PhysicsServer::BODY_MODE_KINEMATIC) {

		_set_transform(new_transform,false);
		_set_inv_transform(new_transform.affine_inverse());
		if (contacts.size()==0 && linear_velocity==Vector3() && angular_velocity==Vector3())
			set_active(false); //stopped moving, deactivate

		return;
	}



	//apply axis lock
	if (axis_lock!=PhysicsServer::BODY_AXIS_LOCK_DISABLED) {


		int axis=axis_lock-1;
		for(int i=0;i<3;i++) {
			if (i==axis) {
				linear_velocity[i]=0;
				biased_linear_velocity[i]=0;
			} else {

				angular_velocity[i]=0;
				biased_angular_velocity[i]=0;
			}
		}

	}


	Vector3 total_angular_velocity = angular_velocity+biased_angular_velocity;



	float ang_vel = total_angular_velocity.length();
	Transform3D transform = get_transform();


	if (ang_vel!=0.0) {
		Vector3 ang_vel_axis = total_angular_velocity / ang_vel;
		Basis rot( ang_vel_axis, -ang_vel*p_step );
		transform.basis = rot * transform.basis;
		transform.orthonormalize();
	}

	Vector3 total_linear_velocity=linear_velocity+biased_linear_velocity;
	/*for(int i=0;i<3;i++) {
		if (axis_lock&(1<<i)) {
			transform.origin[i]=0.0;
		}
	}*/

	transform.origin+=total_linear_velocity * p_step;

	_set_transform(transform);
	_set_inv_transform(get_transform().inverse());

	_update_inertia_tensor();

	//if (fi_callback) {

	//	get_space()->body_add_to_state_query_list(&direct_state_query_list);
	//
}

/*
void BodySW::simulate_motion(const Transform3D& p_xform,real_t p_step) {

	Transform3D inv_xform = p_xform.affine_inverse();
	if (!get_space()) {
		_set_transform(p_xform);
		_set_inv_transform(inv_xform);

		return;
	}

	//compute a FAKE linear velocity - this is easy

	linear_velocity=(p_xform.origin - get_transform().origin)/p_step;

	//compute a FAKE angular velocity, not so easy
	Basis rot=get_transform().basis.orthonormalized().transposed() * p_xform.basis.orthonormalized();
	Vector3 axis;
	float angle;

	rot.get_axis_and_angle(axis,angle);
	axis.normalize();
	angular_velocity=axis.normalized() * (angle/p_step);
	linear_velocity = (p_xform.origin - get_transform().origin)/p_step;

	if (!direct_state_query_list.in_list())// - callalways, so lv and av are cleared && (state_query || direct_state_query))
		get_space()->body_add_to_state_query_list(&direct_state_query_list);
	simulated_motion=true;
	_set_transform(p_xform);


}
*/

void BodySW::wakeup_neighbours() {

	for(Map<ConstraintSW*,int>::Element *E=constraint_map.front();E;E=E->next()) {

		const ConstraintSW *c=E->key();
		BodySW **n = c->get_body_ptr();
		int bc=c->get_body_count();

		for(int i=0;i<bc;i++) {

			if (i==E->get())
				continue;
			BodySW *b = n[i];
			if (b->mode!=PhysicsServer::BODY_MODE_RIGID)
				continue;

			if (!b->is_active())
				b->set_active(true);
		}
	}
}

void BodySW::call_queries() {


	if (fi_callback) {

		PhysicsDirectBodyStateSW *dbs = PhysicsDirectBodyStateSW::singleton;
		dbs->body=this;

		Variant v=dbs;

		Object *obj = ObjectDB::get_instance(fi_callback->id);
		if (!obj) {

			set_force_integration_callback(0,StringName());
		} else {
			const Variant *vp[2]={&v,&fi_callback->udata};

			Variant::CallError ce;
			int argc=(fi_callback->udata.get_type()==Variant::NIL)?1:2;
			obj->call(fi_callback->method,vp,argc,ce);
		}


	}


}


bool BodySW::sleep_test(real_t p_step)  {

	if (mode==PhysicsServer::BODY_MODE_STATIC || mode==PhysicsServer::BODY_MODE_KINEMATIC)
		return true; //
	else if (mode==PhysicsServer::BODY_MODE_CHARACTER)
		return !active; // characters don't sleep unless asked to sleep
	else if (!can_sleep)
		return false;




	if (Math::abs(angular_velocity.length())<get_space()->get_body_angular_velocity_sleep_treshold() && Math::abs(linear_velocity.length_squared()) < get_space()->get_body_linear_velocity_sleep_treshold()*get_space()->get_body_linear_velocity_sleep_treshold()) {

		still_time+=p_step;

		return still_time > get_space()->get_body_time_to_sleep();
	} else {

		still_time=0; //maybe this should be set to 0 on set_active?
		return false;
	}
}


void BodySW::set_force_integration_callback(ObjectID p_id,const StringName& p_method,const Variant& p_udata) {

	if (fi_callback) {

		memdelete(fi_callback);
		fi_callback=NULL;
	}


	if (p_id!=0) {

		fi_callback=memnew(ForceIntegrationCallback);
		fi_callback->id=p_id;
		fi_callback->method=p_method;
		fi_callback->udata=p_udata;
	}

}

BodySW::BodySW() : CollisionObjectSW(TYPE_BODY), active_list(this), inertia_update_list(this), direct_state_query_list(this) {


	mode=PhysicsServer::BODY_MODE_RIGID;
	active=true;

	mass=1;
//	_inv_inertia=Transform3D();
	_inv_mass=1;
	bounce=0;
	friction=1;
	omit_force_integration=false;
//	applied_torque=0;
	island_step=0;
	island_next=NULL;
	island_list_next=NULL;
	first_time_kinematic=false;
	_set_static(false);
	density=0;
	contact_count=0;

	still_time=0;
	continuous_cd=false;
	can_sleep=false;
	fi_callback=NULL;
	axis_lock=PhysicsServer::BODY_AXIS_LOCK_DISABLED;

}

BodySW::~BodySW() {

	if (fi_callback)
		memdelete(fi_callback);
}

PhysicsDirectBodyStateSW *PhysicsDirectBodyStateSW::singleton=NULL;

PhysicsDirectSpaceState* PhysicsDirectBodyStateSW::get_space_state() {

	return body->get_space()->get_direct_state();
}
