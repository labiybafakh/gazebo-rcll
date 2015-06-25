/***************************************************************************
 *  cap_station.h - controls a cap station mps
 *
 *  Generated: Wed Apr 22 12:48:29 2015
 *  Copyright  2015  Randolph Maaßen
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE.GPL_WRE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL_WRE file in the doc directory.
 */

#include "cap_station.h"

using namespace gazebo;

CapStation::CapStation(physics::ModelPtr _parent, sdf::ElementPtr _sdf) :
  Mps(_parent,_sdf)
{
  spawn_puck(shelf_left_pose());
  spawn_puck(shelf_middle_pose());
  spawn_puck(shelf_right_pose());
  workpiece_result_subscriber_ = node_->Subscribe(TOPIC_PUCK_COMMAND_RESULT ,&CapStation::on_puck_result,this);
}

void CapStation::OnUpdate(const common::UpdateInfo &info)
{
  Mps::OnUpdate(info);
  if(model_->GetWorld()->GetSimTime().Double() - created_time_ < SPAWN_PUCK_TIME)
  {
    return;
  }
  if(puck_in_shelf_left_ && !pose_hit(puck_in_shelf_left_->GetWorldPose(),shelf_left_pose(),0.1))
  {
    spawn_puck(shelf_left_pose());
    puck_in_shelf_left_ = NULL;
  }
  if(puck_in_shelf_middle_ && !pose_hit(puck_in_shelf_middle_->GetWorldPose(),shelf_middle_pose(),0.1))
  {
    spawn_puck(shelf_middle_pose());
    puck_in_shelf_middle_ = NULL;
  }
  if(puck_in_shelf_right_ && !pose_hit(puck_in_shelf_right_->GetWorldPose(),shelf_right_pose(),0.1))
  {
    spawn_puck(shelf_right_pose());
    puck_in_shelf_right_ = NULL;
  }
}

void CapStation::on_puck_msg(ConstPosePtr &msg)
{
  if(current_state_ == "READY-AT-OUTPUT")
  {
    if(puck_in_processing_name_ != "" && 
       !puck_in_output(world_->GetModel(puck_in_processing_name_)->GetWorldPose()))
    {
      set_state(State::RETRIEVED);
      puck_in_processing_name_ = "";
    }
  }
  else if(current_state_ == "PREPARED")
  {
    if(puck_in_input(msg))
    {
      set_state(State::AVAILABLE);
      std::string puck_name = msg->name();
      gazsim_msgs::WorkpieceCommand cmd_msg = gazsim_msgs::WorkpieceCommand();
      cmd_msg.set_puck_name(puck_name);
      switch(task_)
      {
        case llsf_msgs::CsOp::RETRIEVE_CAP:
          printf("%s retrives cap from %s\n ", name_.c_str(), puck_name.c_str());
          cmd_msg.set_command(gazsim_msgs::Command::REMOVE_CAP);
          break;
        case llsf_msgs::CsOp::MOUNT_CAP:
          printf("%s mounts cap on %s with color %s\n", name_.c_str(), puck_name.c_str(), gazsim_msgs::Color_Name(stored_cap_color_).c_str());
          cmd_msg.set_command(gazsim_msgs::Command::ADD_CAP);
          cmd_msg.set_color(stored_cap_color_);
          break;
      }
      //set_state(State::PROCESSED);
      puck_cmd_pub_->Publish(cmd_msg);
      world_->GetModel(puck_name)->SetWorldPose(output());
      puck_in_processing_name_ = puck_name;
    }
  }
}

void CapStation::on_new_puck(ConstNewPuckPtr &msg)
{
  Mps::on_new_puck(msg);
  //get model
  physics::Model_V models = world_->GetModels();
  for(physics::ModelPtr model: models)
  {
    if(model->GetName() == msg->puck_name())
    {
      gazsim_msgs::WorkpieceCommand cmd;
      cmd.set_command(gazsim_msgs::Command::ADD_CAP);
      if(name_.find("CS1") != std::string::npos)
      {
        cmd.set_color(gazsim_msgs::Color::GREY);
      }
      else if(name_.find("CS2") != std::string::npos)
      {
        cmd.set_color(gazsim_msgs::Color::BLACK);
      }
      cmd.set_puck_name(msg->puck_name());
      if(pose_in_shelf_left(model->GetWorldPose()))
      {
        puck_in_shelf_left_ = model;
        puck_cmd_pub_->Publish(cmd);
      }
      else if(pose_in_shelf_middle(model->GetWorldPose()))
      {
        puck_in_shelf_middle_ = model;
        puck_cmd_pub_->Publish(cmd);
      }
      else if(pose_in_shelf_right(model->GetWorldPose()))
      {
        puck_in_shelf_right_ = model;
        puck_cmd_pub_->Publish(cmd);
      }
    }
  }
}

void CapStation::new_machine_info(ConstMachine &machine)
{
  if(machine.state() == "PREPARED")
  {
    task_ = machine.instruction_cs().operation();
    printf("%s got a new task: %s\n",name_.c_str(),llsf_msgs::CsOp_Name(task_).c_str());
  }
  else if(machine.state() == "PROCESSED" && puck_in_output(world_->GetModel(puck_in_processing_name_)->GetWorldPose()))
  {
    set_state(State::DELIVERED);
  }
}

void CapStation::on_puck_result(ConstWorkpieceResultPtr &result)
{
  if(result->puck_name() == puck_in_processing_name_)
  {
    printf("%s got cap from %s with color %s\n",name_.c_str(), result->puck_name().c_str(), gazsim_msgs::Color_Name(result->color()).c_str());
    stored_cap_color_ = result->color();
  }
}

math::Pose CapStation::shelf_left_pose()
{
  return get_puck_world_pose(-0.1, 0, BELT_HEIGHT + 0.005);
}

math::Pose CapStation::shelf_middle_pose()
{
  return get_puck_world_pose(-0.2, 0, BELT_HEIGHT + 0.005);
}

math::Pose CapStation::shelf_right_pose()
{
  return get_puck_world_pose(-0.3,0, BELT_HEIGHT + 0.005);
}


bool CapStation::pose_in_shelf_left(const math::Pose &puck_pose)
{
  return pose_hit(puck_pose, shelf_left_pose(),0.05);
}

bool CapStation::pose_in_shelf_middle(const math::Pose &puck_pose)
{
  return pose_hit(puck_pose, shelf_middle_pose(),0.05);
}

bool CapStation::pose_in_shelf_right(const math::Pose &puck_pose)
{
  return pose_hit(puck_pose, shelf_right_pose(),0.05);
}
