//
// Event Dispatcher Abstraction for EPOLL 
//
// Author: Florian Wilkemeyer <fw@f-ws.de>
//
// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
//


#define EPOLL_MAX_PER_CYCLE 10	// Max Events to coalesc. per cycle. 


static int epoll_fd = -1;


void evdp_init(){
		
	epoll_fd = epoll_create( EPOLL_MAX_PER_CYCLE );
	if(epoll_fd == -1){
		ShowFatalError("evdp [EPOLL]: Cannot create event dispatcher (errno: %u / %s)\n", errno, strerror(errno) ); 
		exit(1);
	}
		
}//end: evdp_init()


void evdp_final(){
	
	if(epoll_fd != -1){
		close(epoll_fd);
		epoll_fd = -1;
	}
	
}//end: evdp_final()


int32 evdp_wait(EVDP_EVENT *out_fds, int32 max_events, int32 timeout_ticks){
	struct epoll_event l_events[EPOLL_MAX_PER_CYCLE];
	register struct epoll_event *ev;
	register int nfds, n;
	
	if(max_events > EPOLL_MAX_PER_CYCLE)
		max_events = EPOLL_MAX_PER_CYCLE;
	
	nfds = epoll_wait( epoll_fd,  l_events,		max_events,		timeout_ticks);
	if(nfds == -1){
		// @TODO: check if core is in shutdown mode.  if - ignroe error.
		
		ShowFatalError("evdp [EPOLL]: epoll_wait returned bad / unexpected status (errno: %u / %s)\n", errno, strerror(errno));
		exit(1); //..
	}
	
	// Loop thru all events and copy it to the local ra evdp_event.. struct.
	for(n = 0; n < nfds; n++){
		ev = &l_events[n];
		
		out_fds->fd = ev->data.fd;
		out_fds->events = 0; // clear
		
		if(ev->events & EPOLLHUP)
			out_fds->events |= EVDP_EVENT_HUP;
		
		if(ev->events & EPOLLIN)
			out_fds->events |= EVDP_EVENT_IN;
		
		if(ev->events & EPOLLOUT)
			out_fds->events |= EVDP_EVENT_OUT;
			
		out_fds++;		
	}

	return nfds; // 0 on timeout or > 0  .. 
}//end: evdp_wait()


void evdp_remove(int32 fd,  EVDP_DATA *ep){
	
	if(ep->ev_added == true){
		
		if( epoll_ctl(epoll_fd,  EPOLL_CTL_DEL,  fd,  &ep->ev_data)  != 0){
			ShowError("evdp [EPOLL]: evdp_remove - epoll_ctl (EPOLL_CTL_DEL) failed! fd #%u (errno %u / %s)\n", fd,  errno, strerror(errno));		
		}
		
		ep->ev_data.events = 0; // clear struct.
		ep->ev_data.data.fd = -1; // .. clear struct .. 

		ep->ev_added = false; // not added! 
	}
	

}//end: evdp_remove()


bool evdp_addlistener(int32 fd, EVDP_DATA *ep){
	
	ep->ev_data.events = EPOLLET|EPOLLIN;
	ep->ev_data.data.fd = fd;
	
	// No check here for 'added ?'
	// listeners cannot be added twice.
	//
	if( epoll_ctl(epoll_fd,  EPOLL_CTL_ADD,  fd,  &ep->ev_data) != 0 ){
		ShowError("evdp [EPOLL]: evdp_addlistener - epoll_ctl (EPOLL_CTL_ADD) faield! fd #%u (errno %u / %s)\n", fd, errno, strerror(errno));
		ep->ev_data.events = 0;
		ep->ev_data.data.fd = -1;
		return false;
	}	
	
	ep->ev_added = true;
	
	return true;
}//end: evdp_addlistener()


bool evdp_addclient(int32 fd, EVDP_DATA *ep){
	
	ep->ev_data.events = EPOLLIN | EPOLLHUP;
	ep->ev_data.data.fd = fd;
	
	// No check for "added?" here,
	// this function only gets called upon accpept.
	//
	
	if( epoll_ctl(epoll_fd,  EPOLL_CTL_ADD,  fd, &ep->ev_data) != 0){
		ShowError("evdp [EPOLL]: evdp_addclient - epoll_ctl (EPOLL_CTL_ADD) failed! fd #%u (errno %u / %s)\n", fd, errno, strerror(errno));
		ep->ev_data.events = 0;
		ep->ev_data.data.fd = -1;
		return false;
	}
	
	ep->ev_added = true;
	
	return true;
}//end: evdp_addclient()


bool evdp_addconnecting(int32 fd, EVDP_DATA *ep){
	
	ep->ev_data.events = EPOLLET | EPOLLOUT | EPOLLHUP;
	ep->ev_data.data.fd = fd;
	
	if( epoll_ctl(epoll_fd,  EPOLL_CTL_ADD,  fd, &ep->ev_data) != 0){
		ShowError("evdp [EPOLL]: evdp_addconnecting - epoll_ctl (EPOLL_CTL_ADD) failed! fd #%u (errno %u / %s)\n", fd, errno, strerror(errno));
		ep->ev_data.events = 0;
		ep->ev_data.data.fd = -1; 	
	}
		
	ep->ev_added = true;

	return true;
}//end: evdp_addconnecting()


bool evdp_outgoingconnection_established(int32 fd, EVDP_DATA *ep){
	int32 saved_mask;
	
	if(ep->ev_added != true){
		// ! 
		ShowError("evdp [EPOLL]: evdp_outgoingconnection_established fd #%u is not added to event dispatcher! invalid call.\n", fd);
		return false;
	}
	
	saved_mask = ep->ev_data.events;
	
	ep->ev_data.events = EPOLLIN | EPOLLHUP;
	
	if( epoll_ctl(epoll_fd,  EPOLL_CTL_MOD,  fd, &ep->ev_data) != 0){
		ep->ev_data.events = saved_mask; // restore old mask.
		ShowError("evdp [EPOLL]: evdp_outgoingconnection_established - epoll_ctl (EPOLL_CTL_MOD) failed! fd #%u (errno %u / %s)\n", fd, errno, strerror(errno));
		return false;		
	}
	
	return true;
}//end: evdp_outgoingconnection_established()


bool evdp_writable_add(int32 fd, EVDP_DATA *ep){
	
	if(ep->ev_added != true){
		ShowError("evdp [EPOLL]: evdp_writable_add - tried to add not added fd #%u\n",fd);
		return false;
	}
	
	if(! (ep->ev_data.events  & EPOLLOUT) ){ // 
	
		ep->ev_data.events |= EPOLLOUT;
		if( epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ep->ev_data) != 0 ){
			ShowError("evdp [EPOLL]: evdp_writable_add - epoll_ctl (EPOLL_CTL_MOD) failed! fd #%u (errno: %u / %s)\n", fd, errno, strerror(errno));
			ep->ev_data.events &= ~EPOLLOUT; // remove from local flagmask due to failed syscall.
			return false;
		}
	}
	
	return true;	
}//end: evdp_writable_add()


void evdp_writable_remove(int32 fd, EVDP_DATA *ep){
	
	if(ep->ev_added != true){
		ShowError("evdp [EPOLL]: evdp_writable_remove - tried to remove not added fd #%u\n", fd);
		return;
	}
	
	if( ep->ev_data.events & EPOLLOUT ){
		
		ep->ev_data.events &= ~EPOLLOUT;
		if( epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ep->ev_data) != 0){
			ShowError("evdp [EPOLL]: evdp_writable_remove - epoll_ctl (EPOLL_CTL_MOD) failed! fd #%u (errno %u / %s)\n", fd, errno, strerror(errno));
			ep->ev_data.events |= EPOLLOUT; // add back to local flagmask because of failed syscall.
			return;
		}		
	}
	
	return;	
}//end: evdp_writable_remove()
