
// TODO?

/* void stg_model_lock( stg_model_t* mod ) */
/* { */
/*   pthread_mutex_lock(&mod->mutex); */
/* } */

/* void stg_model_unlock( stg_model_t* mod ) */
/* { */
/*   pthread_mutex_unlock(&mod->mutex); */
/* } */

/* void stg_model_set_property_ex( stg_model_t* mod,  */
/* 				const char* propname,  */
/* 				void* data,  */
/* 				size_t len, */
/* 				stg_property_storage_func_t func ) */
/* { */
/*   assert(mod); */
/*   assert(propname); */
  
/*   // make sure the data and len fields agree */
/*   if( data == NULL ){ assert( len == 0 ); } */
/*   if( len == 0 )    { assert( data == NULL ); } */
  
/*   stg_model_set_property( mod, propname, data, len ); */
  
/*   stg_property_t* prop = g_datalist_get_data( &mod->props, propname ); */
/*   assert(prop); */
/*   prop->storage_func = func; */
/* } */

/* void stg_model_set_property( stg_model_t* mod,  */
/* 			     const char* propname,  */
/* 			     void* data,  */
/* 			     size_t len ) */
/* { */
/*   assert(mod); */
/*   assert(propname); */
  
/*   // make sure the data and len fields agree */
/*   if( data == NULL ){ assert( len == 0 ); } */
/*   if( len == 0 )    { assert( data == NULL ); } */
     
/*   stg_property_t* prop = g_datalist_get_data( &mod->props, propname ); */
  
/*   if( prop == NULL ) */
/*     { */
/*       // create a property and stash it in the model with the right name */
/*       //printf( "* adding model %s property %s size %d\n",  */
/*       //      mod->token, propname, (int)len ); */
      
      
/*       prop = (stg_property_t*)calloc(sizeof(stg_property_t),1);       */
/*       strncpy( prop->name, propname, STG_PROPNAME_MAX ); */
/*       prop->mod = mod; */
      
/*       g_datalist_set_data( &mod->props, propname, (void*)prop ); */
      
/*       // test to make sure the property is there */
/*       stg_property_t* find = g_datalist_get_data( &mod->props, propname ); */
/*       assert( find ); */
/*       assert( find == prop );       */
/*     } */
  
/*   // if there's a special storage function registered, call it */
/*   if( prop->storage_func ) */
/*     { */
/*       //printf( "calling special storage function for model \"%s\" property \"%s\"\n", */
/*       //      prop->mod->token, */
/*       //      prop->name ); */
/*       prop->storage_func( prop, data, len ); */
/*     } */
/*   else */
/*     storage_ordinary( prop, data, len ); */
  
/*   if( prop->callbacks )  */
/*       g_list_foreach( prop->callbacks, stg_property_callback_cb, prop ); */
/* } */


/* int stg_model_add_property_callback( stg_model_t* mod,  */
/* 				     const char* propname,  */
/* 				     stg_property_callback_t callback, */
/* 				     void* user ) */
/* { */
/*   assert(mod); */
/*   assert(propname); */
/*   stg_property_t* prop = g_datalist_get_data( &mod->props, propname ); */
  
  
/*   if( ! prop ) */
/*     { */
/*       PRINT_WARN2( "attempting to add a callback to a nonexistent property (%s:%s)", */
/* 		   mod->token, propname ); */
/*       return 1; // error */
/*     } */
/*   // else */
  
/*   stg_cbarg_t* record = calloc(sizeof(stg_cbarg_t),1); */
/*   record->callback = callback; */
/*   record->arg = user; */
  
/*   prop->callbacks = g_list_append( prop->callbacks, record ); */
  
/*   //printf( "added callback %p data %p (%s)\n", */
/*   //  callback, user, (char*)user ); */

/*   return 0; //ok */
/* } */

/* int stg_model_remove_property_callback( stg_model_t* mod,  */
/* 					const char* propname,  */
/* 					stg_property_callback_t callback ) */
/* { */
/*   stg_property_t* prop = g_datalist_get_data( &mod->props, propname ); */
  
/*   if( ! prop ) */
/*     { */
/*       PRINT_WARN2( "attempting to remove a callback from a nonexistent property (%s:%s)", */
/* 		   mod->token, propname ); */
/*       return 1; // error */
/*     } */
  
/*   // else */

/*   // find our callback in the list of stg_cbarg_t */
/*   GList* el = NULL; */
  
/*   // scan the list for the first matching callback */
/*   for( el = g_list_first( prop->callbacks);  */
/*        el; */
/*        el = el->next ) */
/*     { */
/*       if( ((stg_cbarg_t*)el->data)->callback == callback ) */
/* 	break;       */
/*     } */

/*   if( el ) // if we found the matching callback, remove it */
/*     prop->callbacks = g_list_remove( prop->callbacks, el->data); */
 
/*   if( el && el->data ) */
/*     free( el->data ); */
 
/*   return 0; //ok */
/* } */



/* int stg_model_remove_property_callbacks( stg_model_t* mod, */
/* 					 const char* propname ) */
/* { */
/*   stg_property_t* prop = g_datalist_get_data( &mod->props, propname ); */
  
/*   if( ! prop ) */
/*     { */
/*       PRINT_WARN2( "attempting to remove all callbacks from a nonexistent property (%s:%s)", */
/* 		   mod->token, propname ); */
/*       return 1; // error */
/*     } */
  
/*   // else */
/*   g_list_free( prop->callbacks ); */
/*   prop->callbacks = NULL; */
/*   return 0; //ok */
/* } */


// call a property's callback func
void stg_property_callback_cb( gpointer data, gpointer user )
{
  stg_cbarg_t* cba = (stg_cbarg_t*)data;
  stg_property_t* prop = (stg_property_t*)user;  

  if( ((stg_property_callback_t)cba->callback)( prop->mod, prop->name, prop->data, prop->len, cba->arg ) )
    {
      // the callback returned true, which means we should remove it
      //stg_model_remove_property_callback( prop->mod, prop->name, cba->callback );
    }
}

void stg_property_destroy( stg_property_t* prop )
{
  // empty the list
  if( prop->callbacks )
    g_list_free( prop->callbacks );
  
  if( prop->data )
    free( prop->data );

  free( prop );
}


/* void stg_property_print_cb( GQuark key, gpointer data, gpointer user ) */
/* { */
/*   stg_property_t* prop = (stg_property_t*)data; */
  
/*   printf( "%s key: %s name: %s len: %d func: %p\n",  */
/* 	  user ? (char*)user : "",  */
/* 	  g_quark_to_string(key), 	   */
/* 	  prop->name, */
/* 	  (int)prop->len, */
/* 	  prop->storage_func ); */
/* } */

/* void stg_model_print_properties( stg_model_t* mod ) */
/* { */
/*   printf( "Model \"%s\" has  properties:\n", */
/* 	  mod->token ); */
  
/*   g_datalist_foreach( &mod->props, stg_property_print_cb, "   " ); */
/*   puts( "   <end>" );   */
/* } */


/* void stg_world_add_property_toggles( stg_world_t* world,  */
/* 				     const char* propname,  */
/* 				     stg_property_callback_t callback_on, */
/* 				     void* arg_on, */
/* 				     stg_property_callback_t callback_off, */
/* 				     void* arg_off, */
/* 				     const char* label, */
/* 				     int enabled ) */
/* { */
/*   stg_world_property_callback_args_t* args =  */
/*     calloc(sizeof(stg_world_property_callback_args_t),1); */
  
/*   args->world = world; */
/*   strncpy(args->propname, propname, STG_PROPNAME_MAX ); */
/*   args->callback_on = callback_on; */
/*   args->callback_off = callback_off; */
/*   args->arg_on = arg_on; */
/*   args->arg_off = arg_off; */

/*   gui_add_view_item( propname, label, NULL,  */
/* 		     toggle_property_callback, enabled, args ); */
/* } */



/* void add_callback_wrapper( gpointer key, gpointer value, gpointer user ) */
/* { */
/*   struct cb_package *pkg = (struct cb_package*)user;   */
/*   stg_model_t* mod = (stg_model_t*)value; */
  
/*   size_t dummy; */
/*   if( stg_model_get_property( mod, pkg->propname, &dummy ) ) */
/*     stg_model_add_property_callback( mod, */
/* 				     pkg->propname, */
/* 				     pkg->callback, */
/* 				     pkg->userdata ); */
/* }			    */

/* void remove_callback_wrapper( gpointer key, gpointer value, gpointer user ) */
/* { */
/*   struct cb_package *pkg = (struct cb_package*)user;   */
/*   stg_model_t* mod = (stg_model_t*)value; */
  
/*   size_t dummy; */
/*   if( stg_model_get_property( mod, pkg->propname, &dummy ) ) */
/*     stg_model_remove_property_callback( (stg_model_t*)value, */
/* 					pkg->propname, */
/* 					pkg->callback ); */
/* }			    */
  
/* void stg_world_add_property_callback( stg_world_t* world, */
/* 				      char* propname, */
/* 				      stg_property_callback_t callback, */
/* 				      void* userdata )      */
/* {   */
/*   assert( world ); */
/*   assert( propname ); */
/*   assert( callback ); */

/*   struct cb_package pkg; */
/*   pkg.propname = propname; */
/*   pkg.callback = callback; */
/*   pkg.userdata = userdata; */

/*   g_hash_table_foreach( world->models, add_callback_wrapper, &pkg ); */
/* } */

/* void stg_world_remove_property_callback( stg_world_t* world, */
/* 					 char* propname, */
/* 					 stg_property_callback_t callback ) */
/* {   */
/*   assert( world ); */
/*   assert( propname ); */
/*   assert( callback ); */
  
/*   struct cb_package pkg; */
/*   pkg.propname = propname; */
/*   pkg.callback = callback; */
/*   pkg.userdata = NULL; */

/*   g_hash_table_foreach( world->models, remove_callback_wrapper, &pkg ); */
/* } */

